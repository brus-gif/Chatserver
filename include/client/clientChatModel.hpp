// 客户端功能模块
#ifndef CLIENTCHATMODEL_H
#define CLIENTCHATMODEL_H
#include "json.hpp"
#include "public.hpp"
#include "user.hpp"
#include <arpa/inet.h>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
using json = nlohmann::json;

class ClientChatModel
{
  public:
    // 显示所有支持的命令 格式help
    void help(int, std::string);
    // 一对一聊天，格式 chat:friendid:message
    void chat(int fd, std::string str);
    // 添加好友，格式 addfriend:friendid
    void addfriend(int fd, std::string str);
    // 创建群，格式 creategroup:groupname:groupdesc
    void creategroup(int fd, std::string str);
    // 加入群，格式 addgroup:groupid
    void addgroup(int fd, std::string str);
    // 群聊，格式 groupchat:groupid:message
    void groupchat(int fd, std::string str);
    // 注销，格式 loginout
    void loginout(int fd, std::string str);
    // 获得初始化的单例对象
    static ClientChatModel *getInstance(int id = -1, std::string name = "", std::string pwd = "",
                                        std::string state = "offline");
    // 获取命令映射
    std::unordered_map<std::string, std::string> &getCommandMap()
    {
        return commandMap;
    }

    // 获取服务器发来的消息的处理函数(存储形式：通信套接字，消息内容)
    std::unordered_map<std::string, std::function<void(int, std::string)>> &getHandlerMap()
    {
        return handlerMap;
    }
    // 获取当前时间
    std::string getNowTime()
    {
        time_t t = time(nullptr);
        tm *p = localtime(&t);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", p);
        return buf;
    }

  private:
    ClientChatModel(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline");
    std::unordered_map<std::string, std::string> commandMap; // 存呈现给用户的命令
    std::unordered_map<std::string, std::function<void(int, std::string)>>
        handlerMap; // 存储服务器发来的消息的处理函数(存储形式：通信套接字，消息内容)
    User current_user;
};

#endif