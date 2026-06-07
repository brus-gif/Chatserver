// 客户端功能模块
#include "clientChatModel.hpp"

// 获得初始化的单例对象
ClientChatModel *ClientChatModel::getInstance(int id, std::string name, std::string pwd, std::string state)
{
    static ClientChatModel model(id, name, pwd, state);
    return &model;
}

// 初始化存储回调函数的容器，使用绑定器绑定
ClientChatModel::ClientChatModel(int id, std::string name, std::string pwd, std::string state)
    : current_user(id, name, pwd, state)
{
    commandMap.insert({"help", "显示所有支持的命令 格式help"});
    commandMap.insert({"chat", "一对一聊天，格式 chat:friendid:message"});
    commandMap.insert({"addfriend", "添加好友，格式 addfriend:friendid"});
    commandMap.insert({"creategroup", "创建群，格式 creategroup:groupname:groupdesc"});
    commandMap.insert({"addgroup", "加入群，格式 addgroup:group id"});
    commandMap.insert({"groupchat", "群聊，格式 groupchat:groupid:message"});
    commandMap.insert({"loginout", "注销，格式 loginout"});
    handlerMap.insert({"help", std::bind(&ClientChatModel::help, this, std::placeholders::_1, std::placeholders::_2)});
    handlerMap.insert({"chat", std::bind(&ClientChatModel::chat, this, std::placeholders::_1, std::placeholders::_2)});
    handlerMap.insert(
        {"addfriend", std::bind(&ClientChatModel::addfriend, this, std::placeholders::_1, std::placeholders::_2)});
    handlerMap.insert(
        {"creategroup", std::bind(&ClientChatModel::creategroup, this, std::placeholders::_1, std::placeholders::_2)});
    handlerMap.insert(
        {"addgroup", std::bind(&ClientChatModel::addgroup, this, std::placeholders::_1, std::placeholders::_2)});
    handlerMap.insert(
        {"groupchat", std::bind(&ClientChatModel::groupchat, this, std::placeholders::_1, std::placeholders::_2)});
    handlerMap.insert(
        {"loginout", std::bind(&ClientChatModel::loginout, this, std::placeholders::_1, std::placeholders::_2)});
}

// 显示所有支持的命令 格式help
void ClientChatModel::help(int fd, std::string str)
{
    std::cout << "show command list that you can use:" << std::endl;
    for (auto &command : commandMap)
    {
        std::cout << command.first << "  " << command.second << std::endl;
    }
    std::cout << std::endl;
}

// 一对一聊天，格式 chat:friendid:message
void ClientChatModel::chat(int fd, std::string str)
{
    int dex = str.find(":");
    if (dex == std::string::npos)
    {
        // 没找到
        std::cerr << "client:invalid args! the chat command format is chat:friendid:message" << std::endl;
        return;
    }
    int friendid = atoi(str.substr(0, dex).c_str());
    str = str.substr(dex + 1);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["time"] = getNowTime();
    js["id"] = current_user.getId();
    js["name"] = current_user.getName();
    js["to"] = friendid;
    js["msg"] = str;
    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        if (errno != EAGAIN)
        {
            std::cerr << "client send chat msag error! the buffer you send:" << js.dump().c_str() << std::endl;
        }
    }
}

// 添加好友，格式 addfriend:friendid
void ClientChatModel::addfriend(int fd, std::string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIENDM_MSG;
    js["id"] = current_user.getId();
    js["friendid"] = friendid;
    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        if (errno != EAGAIN)
        {
            std::cerr << "client send addfriend msg error! the buffer you send:" << js.dump().c_str() << std::endl;
        }
    }
}

// 创建群，格式 creategroup:groupname:groupdesc
void ClientChatModel::creategroup(int fd, std::string str)
{
    int dex = str.find(":");
    if (dex == std::string::npos)
    {
        std::cerr << "client :invalid args! the creategroup command format is creategroup:groupname:groupdesc"
                  << std::endl;
        return;
    }
    std::string name = str.substr(0, dex);
    std::string groupdesc = str.substr(dex + 1);
    json js;
    js["id"] = current_user.getId();
    js["msgid"] = CREATE_GROUP_MSG;
    js["groupname"] = name;
    js["groupdesc"] = groupdesc;
    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        if (errno != EAGAIN)
        {
            std::cerr << "client send creategroup msg error! the buffer you send:" << js.dump().c_str() << std::endl;
        }
    }
}

// 加入群，格式 addgroup:groupid
void ClientChatModel::addgroup(int fd, std::string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = current_user.getId();
    js["groupid"] = groupid;
    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        if (errno != EAGAIN)
        {
            std::cerr << "client send addgroup error! the buffferyou send:" << js.dump().c_str() << std::endl;
        }
    }
}

// 群聊，格式 groupchat:groupid:message
void ClientChatModel::groupchat(int fd, std::string str)
{
    int dex = str.find(":");
    if (dex == std::string::npos)
    {
        // 没找到groupid
        std::cerr << "client :invalid args! the groupchat command format is groupchat:groupid:message" << std::endl;
        return;
    }
    int groupid = atoi(str.substr(0, dex).c_str());
    std::string message = str.substr(dex + 1);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["time"] = getNowTime();
    js["id"] = current_user.getId();
    js["name"] = current_user.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        if (errno != EAGAIN)
        {
            std::cerr << "client send groupchat error! the buffer you send:" << js.dump().c_str() << std::endl;
        }
    }
}

// 注销，格式 loginout
void ClientChatModel::loginout(int fd, std::string str)
{
    std::cout << "enter login" << std::endl;
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = current_user.getId();
    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        if (errno != EAGAIN)
        {
            std::cerr << "client send loginout error! the buffer you send :" << js.dump() << std::endl;
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 沉睡一秒
    }
}
