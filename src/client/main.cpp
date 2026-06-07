#include "clientserver.hpp"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "command invalid ! example: ./ChatClient 127.0.0.1 20009" << std::endl;
        exit(-1);
    }
    Client client;
    int clientfd;
    while (1)
    {
        if ((clientfd = client.socket_connect(argv[1], atoi(argv[2]))) == -1) // 建立与服务器的连接
        {
            perror("connect server error\n");
            exit(-1);
        }
        // 主页面展示
        // std::cout << "clientfd in main:" << clientfd << std::endl;
        client.showWind(clientfd);
        std::cout << "try to connect again" << std::endl;
    }
    return 0;
}