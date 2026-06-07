// 客户端本地服务模块
#ifndef CLIENTSEVRER_H
#define CLIENTSERVER_H
#include "json.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
using json = nlohmann::json;
#include "group.hpp"
#include "groupuser.hpp"
#include "public.hpp"
#include "user.hpp"
#include <atomic>
#include <errno.h>
class Client
{
  public:
    // 登陆后的用户信息（好友列表，群组）
    void showCurrentUserdata();

    // 接收消息
    void recvTaskhandler(int clienfd);

    // 主聊天页面程序
    void mainMenu(int clientfd);

    // 只与服务器进行交互，接收服务器发来的消息并进行解析,
    // 网络层接口,返回通信套接字
    int socket_connect(char *ip, uint16_t port);

    // 展示主菜单页面
    void showWind(int fd);

    // 客户端登录业务处理
    bool ClientLogin(int fd, int id, char *pwd);

    // 客户端注册业务处理
    bool Clientregister(int fd, char *name, char *pwd);

  private:
    User current_user;                      // 当前登录用户（id，name，password，state）
    std::vector<Group> current_Group_list;  // 当前登录用户的群组列表
    std::vector<User> current_friends_list; // 当前登陆用户的好友列表
    // std::thread recvMsg;
    std::atomic_bool backflag = true; // 与服务器连接是否正常
};

#endif