// 客户端本地服务模块
#include "clientserver.hpp"
#include "clientChatModel.hpp"
// 连接服务器
int Client::socket_connect(char *ip, uint16_t port)
{
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("socket create fail");
        std::cout << std::endl;
        exit(-1);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if ((inet_pton(AF_INET, ip, &server_addr.sin_addr)) == -1)
    {
        perror("pton error");
        std::cout << std::endl;
        exit(-1);
    }
    if (!connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        return clientfd;
    }
    return -1;
}

// 接收消息线程函数
void Client::recvTaskhandler(int clientfd) // 这里由于服务没有设置建群成功/加好友成功等消息响应，所以这块里暂不实现
{
    while (1)
    {
        char buf[2048] = {0};
        int len = recv(clientfd, buf, sizeof(buf), 0);
        if (len == 0)
        {
            close(clientfd);
            std::cerr << "the server error,please return to the homepage and log in again!" << std::endl;
            exit(-1); // 这里没有使用线程间的通信所以直接退出
        }
        else if (len == -1)
        {
            if (errno != EAGAIN)
            {
                close(clientfd);
                std::cerr << "the recvTaskhandler recv error!" << std::endl;
                exit(-1); // 这里没有使用线程间的通信所以直接退出
            }
        }

        json js = json::parse(buf);                 // 反序列化
        if (js["msgid"].get<int>() == ONE_CHAT_MSG) // 一对一聊天信息接收
        {

            std::cout << "One chat message:" << js["time"].get<std::string>() << " " << js["id"].get<int>() << " "
                      << js["name"].get<std::string>() << " " << "said:" << js["msg"].get<std::string>() << std::endl;
            continue;
        }
        else if (js["msgid"].get<int>() == GROUP_CHAT_MSG) // 群聊接收
        {
            std::cout << "group message:[" << js["groupid"].get<int>() << "] " << js["time"].get<std::string>() << " "
                      << js["id"].get<int>() << " " << js["name"].get<std::string>() << " "
                      << "said:" << js["msg"].get<std::string>() << std::endl;
            continue;
        }
        else if (js["msgid"].get<int>() == LOGINOUT_MSG_ACK)
        {
            backflag = false;
            std::cout << "recvhandlerthread exit" << std::endl;
            break;
        }
    }
}

// 进入聊天主页面
void Client::mainMenu(int clientfd)
{
    ClientChatModel *clientChatModel = ClientChatModel::getInstance(current_user.getId(), current_user.getName(),
                                                                    current_user.getPwd(), current_user.getState());
    clientChatModel->help(clientfd, "help");
    // char buf[2048] = {0};
    while (backflag) // 在连接没断开的前提下正常使用
    {
        std::string str;
        getline(std::cin, str);
        if (!backflag)
        {
            break;
        }
        std::string command; // 命令
        int dex = str.find(":");
        if (dex == std::string::npos) // 找不到“：”说明用户输入的命令无参数
        {
            command = str;
        }
        else // 找到了“：”，将命令单独提取出来
        {
            command = str.substr(0, dex);
        }
        auto it = clientChatModel->getHandlerMap().find(command); // 查找该命令否合法
        if (it == clientChatModel->getHandlerMap().end())
        {
            // 不合法，重新输入
            std::cerr << "invalid command!" << std::endl;
            continue;
        }
        str = str.substr(dex + 1); // 提取命令参数
        it->second(clientfd, str); // 合法，调用对应的回调函数
    }
    std::cout << "client loginout success! please back mainpage" << std::endl;
}

// 打印登录用户的群组列表和好友列表
void Client::showCurrentUserdata()
{
    std::cout << "=================login user================" << std::endl;
    std::cout << "current login user id:" << current_user.getId() << "  " << "name:" << current_user.getName()
              << std::endl;

    std::cout << "====================friend list===================" << std::endl;
    if (!current_friends_list.empty())
    {
        for (User &user : current_friends_list)
        {
            std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
        }
    }
    std::cout << "=================group list===================" << std::endl;
    if (!current_Group_list.empty())
    {
        for (Group &group : current_Group_list)
        {
            std::cout << group.getId() << "    " << group.getName() << "  " << group.getDesc() << std::endl;
            std::cout << "the member of this group:" << std::endl;
            for (GroupUser &guser : group.getUsers())
            {
                std::cout << guser.getId() << "  " << guser.getName() << "  " << guser.getState() << "  "
                          << guser.getRole() << std::endl;
            }
        }
    }

    std::cout << "============================" << std::endl;
}

// 客户端注册业务
bool Client::Clientregister(int fd, char *name, char *pwd)
{
    json js;
    js["msgid"] = REG_MSG;
    js["name"] = name;
    js["password"] = pwd;
    std::string request = js.dump();
    int len = send(fd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1) // 发送注册请求失败
    {
        perror("register send error!");
        return false;
    }
    else // 发送注册请求成功
    {
        char buf[1024] = {0};
        len = recv(fd, buf, sizeof(buf), 0);
        if (len == -1) // 获取服务器回应失败
        {
            perror("register recv error! ");
            return false;
        }
        else if (len == 0) // 服务器异常断开
        {
            perror("the server interrupt the connection! Please register in again shortly.");
            return false;
        }
        else // 成功获取服务器回应
        {
            json registerjs = json::parse(buf);
            if (registerjs["errno"].get<int>() == 0 && registerjs["msgid"] == REG_MSG_ACK) // 注册成功
            {
                std::cout << "your id:" << registerjs["id"] << std::endl;
            }
            else // 注册失败
            {
                std::cout << registerjs["errmsg"] << std::endl;
                return false;
            }
        }
    }
    return true;
}

// 客户端登录业务
bool Client::ClientLogin(int fd, int id, char *pwd)
{
    std::cout << "fd in ClientLogin:" << fd << std::endl;

    json js;
    js["msgid"] = LOGIN_MSG;
    js["id"] = id;
    js["password"] = pwd;
    std::string request = js.dump();
    int len =
        send(fd, request.c_str(), strlen(request.c_str()) + 1, 0); // 向一个服务器发送，接收有另一个线程，所以这里就阻塞
    if (len == -1)
    {
        perror("send login error!");
        return false;
    }
    else // 发送成功，尝试获取服务器的响应
    {
        char buff[2048] = {0};
        len = recv(fd, buff, sizeof(buff), 0);
        if (len == -1)
        {
            perror("client login recv error! \n");
            return false;
        }
        else if (len == 0) // 服务器异常断开
        {
            std::cerr << "the server interrupt the connection! Please log in again shortly." << std::endl;
            return false;
        }
        else // 收到了服务器返回的响应
        {
            json response = json::parse(buff);
            if (response["msgid"] == LOGIN_MSG_ACK && response["errno"].get<int>() != 0) // 登陆失败
            {
                std::cerr << response["errmsg"] << std::endl;
                return false;
            }
            else // 登陆成功
            {

                current_user.setId(response["id"].get<int>());
                current_user.setName(response["name"]);
                // current_user.setState("online");

                // 检查当前用户有没有好友，也就是是否有好友列表返回客户端
                if (response.contains("friendlist"))
                {
                    // 有好友列表
                    current_friends_list.clear(); // 清除前一位用户的信息
                    std::vector<std::string> vec = response["friendlist"];
                    for (std::string &str : vec)
                    {
                        json friendjs = json::parse(str); // 反序列化每一个User类型的好友信息
                        User fuser;
                        fuser.setId(friendjs["id"].get<int>());
                        fuser.setName(friendjs["name"]);
                        fuser.setState(friendjs["state"]);
                        current_friends_list.push_back(fuser);
                    }
                }

                // 检查当前用户有没有加群
                if (response.contains("groups"))
                {
                    // 当前用户加群了
                    current_friends_list.clear(); // 清除前一位用户的信息
                    std::vector<std::string> vec = response["groups"];
                    for (std::string &str : vec)
                    {
                        json groupjs = json::parse(str);
                        Group groupl;
                        groupl.setId(groupjs["groupid"].get<int>());
                        groupl.setName(groupjs["groupname"]);
                        groupl.setDesc(groupjs["groupdesc"]);
                        std::vector<std::string> groupusers = groupjs["groupusers"];
                        for (std::string &groupUser : groupusers)
                        {
                            json Users = json::parse(groupUser);
                            GroupUser groupusers;
                            groupusers.setId(Users["id"].get<int>());
                            groupusers.setName(Users["name"]);
                            groupusers.setState(Users["state"]);
                            groupusers.setRole(Users["role"]);
                            groupl.getUsers().push_back(groupusers);
                        }
                        current_Group_list.push_back(groupl);
                    }
                }

                // 查看离线列表中有没有离线消息,把离线消息显示给用户
                if (response.contains("offlinemsg"))
                {
                    std::vector<std::string> vec = response["offlinemsg"];
                    for (std::string &str : vec)
                    {
                        json js = json::parse(str);
                        std::cout << js["time"] << "  " << js["id"] << "  " << js["name"] << " " << "said:" << js["msg"]
                                  << std::endl;
                    }
                }
                // 登陆完成，启动接收消息线程接收数据
                // recvMsg=std::thread (std::bind(&Client::recvTaskhandler, this, fd));
                showCurrentUserdata();
                static int recvthreadnum = 0;
                // if (recvthreadnum == 0) // 线程只启动一次
                // {
                std::cout << "start recvtaskhandler thread" << std::endl;
                std::thread recvd(
                    std::bind(&Client::recvTaskhandler, this, fd)); // 移动赋值，线程生命周期与当前对象周期绑定
                recvd.detach();                                     // 分离线程
                // recvthreadnum++;
                // }
                // 进入聊天主页面
                backflag = true;
                mainMenu(fd);
            }
        }
    }
    return true;
}
// 页面展示和功能选择
void Client::showWind(int fd)
{
    // std::cout << "fd in showWind:" << fd << std::endl;

    while (1)
    {
        // 展示主页菜单  登录，注册，退出
        std::cout << "1.login" << std::endl;
        std::cout << "2.register" << std::endl;
        std::cout << "3.exit" << std::endl;
        std::cout << "choice:" << std::endl;

        int choice;
        std::cin >> choice;
        std::cin.get(); // 清空缓冲区的\n

        // 选择对应的服务
        switch (choice)
        {
        case 1: // login登录业务
        {
            int id;       // 用户id
            char pwd[50]; // 用户密码
            std::cout << "your userid:" << std::endl;
            std::cin >> id;
            std::cin.get();
            std::cout << "your password:" << std::endl;
            std::cin.getline(pwd, 50);
            std::cout << "fd in case 1:" << fd << std::endl;
            ClientLogin(fd, id, pwd);
            continue;
        }
        break;
        case 2: // resgister注册业务
        {
            char name[50] = {0}; // 用户id
            char pwd[50] = {0};  // 用户密码
            std::cout << "username:" << std::endl;
            std::cin.getline(name, 50);
            std::cout << "password:" << std::endl;
            std::cin.getline(pwd, 50);
            Clientregister(fd, name, pwd);
            continue;
        }
        break;
        case 3: // 退出业务
        {
            close(fd); // 关闭通信套接字
            exit(0);
        }
        break;
        default:
            std::cerr << "invalid input!" << std::endl;
            break;
        }
    }
}
