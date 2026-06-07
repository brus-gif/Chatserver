// 网络模块代码
#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <string>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;
//聊天服务器的主类
class Chatserver
{
public:
Chatserver(EventLoop*loop,const InetAddress&listenAddr,const string &nameArg);
//开启服务
void start();



private:
void onMessage(const TcpConnectionPtr&,Buffer*,Timestamp time);
void onconnection(const TcpConnectionPtr &);

    TcpServer _server;
    EventLoop*_loop;
};
#endif