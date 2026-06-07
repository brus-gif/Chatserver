// 业务模块代码实现
#include "chatservice.hpp"
#include "public.hpp"
using namespace std::placeholders;
#include <vector>
// 业务模块单例构造函数
Chatservice *Chatservice::instance()
{
    static Chatservice service;
    return &service;
}

// 登录事务
void Chatservice::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    std::string pwd = js["password"];
    User user = _usermodel.query(id);
    if (user.getPwd() == pwd) // 用户密码正确
    {
        json response;
        // 用户已经登陆，不允许重复登录
        if (user.getState() == "online")
        {
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "This account has already been logged in on another device!";
            conn->send(response.dump());
        }
        else
        {
            // 登陆成功，更新状态
            user.setState("online");
            if (!_usermodel.updatestate(user)) // 状态更新失败
            {
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 3;
                response["errmsg"] = "the update of user's state is failed! ";
                conn->send(response.dump());
                return;
            }

            // 状态更新成功，登陆完成
            // 记录用户连接

            redis.subscribe(id);

            {
                std::lock_guard<std::mutex> lock(connmutex);
                _UserConnmap.emplace(id, conn);
            }
            // 给客户端返回响应
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            // 给客户端返回id和昵称
            response["id"] = id;
            response["name"] = user.getName();

            // 查询用户是否有离线消息
            std::vector<std::string> vec = _offlineMsg.query(id);
            if (!vec.empty())
            {

                response["offlinemsg"] = vec;
                // 删除获得离线消息的用户对应数据库上的离线消息
                _offlineMsg.remove(id);

            } // 如果该用户没有离线消息，就不发送离线消息列表了

            // 查询用户的好友列表信息并转发
            std::vector<User> frinendlist = _friendmodel.query(id);
            if (!frinendlist.empty()) // 用户列表有好友
            {
                std::vector<std::string> v;
                for (User &users : frinendlist)
                {
                    json js;
                    js["id"] = users.getId();
                    js["name"] = users.getName();
                    js["state"] = user.getState();
                    v.push_back(js.dump());
                }
                response["friendlist"] = v;
            } // 该用户没有任何好友就不发送好友列表了

            // 查询用户的群组列表信息并转发
            std::vector<Group> group = _groupModel.queryGroups(id); // 获得用户组及组中成员信息后对这些信息序列化
            if (!group.empty())                                     // 用户有对应的用户组
            {
                // 分别把Group对象和GroupUser对象都序列化，后者序列化放到vector<string>再赋给前者的js
                std::vector<std::string> Groups; // 存每个群组的信息和他们组员的信息（序列化后的）
                for (Group &groupv : group)
                {
                    json resjs;
                    resjs["groupid"] = groupv.getId();
                    resjs["groupname"] = groupv.getName();
                    resjs["groupdesc"] = groupv.getDesc();

                    std::vector<std::string> Groupusers;
                    for (GroupUser &groupuser : groupv.getUsers())
                    {
                        json users;
                        users["id"] = groupuser.getId();
                        users["name"] = groupuser.getName();
                        users["state"] = groupuser.getState();
                        users["role"] = groupuser.getRole();
                        Groupusers.push_back(users.dump());
                    }

                    resjs["groupusers"] = Groupusers;
                    Groups.push_back(resjs.dump());
                }
                response["groups"] = Groups;
            }
            // 该用户没有加入任何群组就发送群组信息了
            conn->send(response.dump());
        }
    }
    else // 登陆失败
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "the password is wrong!";
        conn->send(response.dump());
    }
}

// 注册事务
void Chatservice::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];
    User user;
    user.setPwd(pwd);
    user.setName(name);
    if (_usermodel.insert(user)) // 注册成功
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else // 注册失败
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "register fail,the name is exit";
        conn->send(response.dump());
    }
}
// 注销登录业务
void Chatservice::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connmutex);
        auto it = _UserConnmap.find(userid);
        if (it != _UserConnmap.end())
        {
            _UserConnmap.erase(it); // 删除用户的本地连接
        }

    } // 更新用户在数据库中记录的状态
    redis.unsubscribe(userid);

    User user(userid, "", "", "offline");
    std::cout << "loginout userstate:" << user.getState() << std::endl;
    _usermodel.updatestate(user);
    json response;
    response["msgid"] = LOGINOUT_MSG_ACK;
    conn->send(response.dump());
}

void Chatservice::handlerRedisMessage(int userid, std::string msg)
{
    std::cout << "handlermsg recive:" << msg << std::endl;
    json js = json::parse(msg.c_str());
    {
        lock_guard<std::mutex> lock(connmutex);
        auto it = _UserConnmap.find(userid);
        if (it != _UserConnmap.end())
        {
            it->second->send(js.dump());
            return;
        }
    }
    _offlineMsg.insert(userid, msg);
}

// 注册事务回调函数
Chatservice::Chatservice()
{
    // 注册登录事件
    _msghandlerMap.emplace(LOGIN_MSG, std::bind(&Chatservice::login, this, _1, _2, _3));
    // 注册注册事件
    _msghandlerMap.emplace(REG_MSG, std::bind(&Chatservice::reg, this, _1, _2, _3));
    // 注册一对一聊天事件
    _msghandlerMap.emplace(ONE_CHAT_MSG, std::bind(&Chatservice::oneChat, this, _1, _2, _3));
    // 注册添加好友事件
    _msghandlerMap.emplace(ADD_FRIENDM_MSG, std::bind(&Chatservice::addFriend, this, _1, _2, _3));
    // 注册创建群组事件
    _msghandlerMap.emplace(CREATE_GROUP_MSG, std::bind(&Chatservice::createGroup, this, _1, _2, _3));
    // 注册加入群组事件
    _msghandlerMap.emplace(ADD_GROUP_MSG, std::bind(&Chatservice::addGroup, this, _1, _2, _3));
    // 注册群聊事件
    _msghandlerMap.emplace(GROUP_CHAT_MSG, std::bind(&Chatservice::groupChat, this, _1, _2, _3));
    // 注册注销登录事件
    _msghandlerMap.emplace(LOGINOUT_MSG, std::bind(&Chatservice::loginout, this, _1, _2, _3));

    if (redis.connect())
    {
        std::cout << "redis connect" << std::endl;
        redis.init_notify_handler(std::bind(&Chatservice::handlerRedisMessage, this, _1, _2));
    }
}

// 通过对应的MSG序号得到对应的事务回调函数
Msghandler Chatservice::gethandler(int msgid)
{
    auto it = _msghandlerMap.find(msgid);
    // 没找到对应的事务回调函数
    if (it == _msghandlerMap.end())
    {
        return [&](const TcpConnectionPtr &conn, json &js, Timestamp time) -> void {
            LOG_ERROR << "error:" << msgid << "is not exist";
        };
    }
    else
    {
        // 返回对应的事务回调函数
        return _msghandlerMap[msgid];
    }
}

// 处理由于客户端异常断开连接
void Chatservice::clientCloseException(const TcpConnectionPtr &conn)
{
    std::cout << "enter clientCloseException" << std::endl;
    User user;
    {
        // 把该用户在本地的连接记录删除
        std::lock_guard<std::mutex> lock(connmutex);
        for (auto it = _UserConnmap.begin(); it != _UserConnmap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                _UserConnmap.erase(it);
                break;
            }
        }
    }
    // 更新数据库中用户状态
    user.setState("offline");
    std::cout << "the old state:" << user.getState() << std::endl;
    _usermodel.updatestate(user);
}

// 一对一聊天函数，服务器转发消息给目标用户js["id"],js["from"],js["to"],js["msg"]
void Chatservice::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // std::cout << "enter one Chat" << std::endl;
    int toid = js["to"];

    {
        std::lock_guard<std::mutex> lock(connmutex);
        auto it = _UserConnmap.find(toid);
        // 查用户在不在线
        if (it != _UserConnmap.end())
        { // 用户在线
            std::cout << "message send" << std::endl;
            it->second->send(js.dump()); // 服务器给在线的用户转发消息
            return;
        }
    }

    User user = _usermodel.query(toid);
    if (user.getState() == "online")
    {
        redis.publish(toid, js.dump());
        return;
    }

    // 用户离线，服务器给离线用户转发离线消息
    _offlineMsg.insert(toid, js.dump()); // 发送的是json序列化，因为接收方通过解析json得知发送方信息
}

// 处理服务器异常断开连接的情况——将数据库中的User表中的用户状态重置
void Chatservice::reset()
{
    _usermodel.resetState();
}

// 好友添加的业务模块
void Chatservice::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    // 加入好友列表，插入数据库好友表
    _friendmodel.insert(userid, friendid);
}

// 创建群组业务：
void Chatservice::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{ // 首先，::ALLGroup的主键自增，只需要插入群组名称grouname和群组描述groupdesc就可以了，数据库返回自增主键
    // 其次，要将当前用户加入群组并设置为群主
    int userid = js["id"].get<int>();
    std::string groupname = js["groupname"];
    std::string groupdesc = js["groupdesc"];
    Group group(-1, groupname, groupdesc);
    if (_groupModel.CreateGroup(group)) // 群组创建成功
    {
        _groupModel.addGroup(userid, group.getId(), "creator"); // 将创建者加入群组，并设置角色为creator
        // 给客户端成功的响应
    }
}

// 加入群组业务：
void Chatservice::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal"); // 将加入者加入群组，并设置角色为normal

    // 给客户端成功的响应
}

// 群聊业务:js["id"],js["time"],js["name"],js["groupid"](这个不打印出来),js["msg"]
void Chatservice::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> groupuser =
        _groupModel.queryGroupUsers(userid, groupid); // 查询群组用户的id列表，主要用于群聊业务，发消息给群中的成员
                                                      // 发起群聊，给在线用户转发消息，离线用户存储离线消息
    {
        std::lock_guard<std::mutex> lock(connmutex); // 锁粒度大，待优化点
        for (int id : groupuser)
        {

            auto it = _UserConnmap.find(id);
            if (it != _UserConnmap.end()) // 用户在线
            {
                it->second->send(js.dump()); // 服务器给在线的用户转发消息
            }
            else
            {
                User user = _usermodel.query(id);
                if (user.getState() == "online")
                {
                    redis.publish(id, js.dump());
                }
                else
                {                                      // 用户离线，服务器给离线用户转发离线消息
                    _offlineMsg.insert(id, js.dump()); // 发送的是json序列化，因为接收方通过解析json得知发送方信息)
                }
            }
        }
    }
}