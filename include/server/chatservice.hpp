// 业务模块  ——实现网络模块与业务模块的解耦
#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "json.hpp"
#include "offlinemessagemodel.hpp"
#include "redis.hpp"
#include "usermodel.hpp"
#include <functional>
#include <iostream>
#include <muduo/base/Logging.h>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include <unordered_map>
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
using Msghandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;
class Chatservice
{
  public:
    // 获取单例对象
    static Chatservice *instance();
    // 登陆业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注册业务：
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注销登录业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获得对应事务的回调函数
    Msghandler gethandler(int msgid);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    // 处理一对一聊天业务(服务器只负责转发数据)
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理服务器异常退出
    void reset();

    // 添加好友业务：
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组业务：
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组业务：
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群聊业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 从redis消息队列中获得订阅消息
    void handlerRedisMessage(int, std::string);

  private:
    // 饿汉式单例模式
    // static Chatservice service;
    // 这里使用懒汉式单例模式
    Chatservice();
    Chatservice(Chatservice &) = delete;
    Chatservice operator=(Chatservice &) = delete;

    // 存放不同业务回调函数的键值对
    std::unordered_map<int, Msghandler> _msghandlerMap;

    // 建立数据库映射模块的对象
    Usermodel _usermodel;

    // 存放不同连接的键值对，更好的管理连接（使业务层有能力主动控制连接）
    // 只存在线的用户的连接（只管理有注册，已登陆用户）
    std::unordered_map<int, TcpConnectionPtr> _UserConnmap;

    // 保证连接访问的线程安全
    std::mutex connmutex;

    // 建立离线消息模块对象
    OfflineMsgModel _offlineMsg;

    // 建立好友表映射模块对象
    FriendModel _friendmodel;

    // 建立群组表映射模块对象
    GroupModel _groupModel;
    // redis对象
    Redis redis;
};

#endif