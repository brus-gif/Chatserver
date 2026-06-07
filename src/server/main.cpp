#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
void resetstatehandler(int signum)
{
    Chatservice::instance()->reset();
    // 这里为了程序正常退出设置了exit，如果真实业务不需要退出
    exit(0);
}

int main(int argc, char *argv[])
{
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    struct sigaction act;
    act.sa_handler = resetstatehandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER;
    sigaction(SIGINT, &act, nullptr);
    EventLoop loop;
    InetAddress addr(ip, port);

    Chatserver server(&loop, addr, "Myserver");
    // 开启网络模块的相关事务
    server.start();

    // 开启事件循环
    loop.loop();

    return 0;
}