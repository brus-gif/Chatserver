//网络模块代码的实现
#include"chatserver.hpp"
#include"json.hpp"
#include"chatservice.hpp"
#include<functional>
using json=nlohmann::json;
using namespace std::placeholders;
//网络模块对象构造函数
Chatserver::Chatserver(EventLoop*loop,const InetAddress&listenAddr,const string &nameArg)
:_server(loop,listenAddr,nameArg),_loop(loop)
{
_server.setConnectionCallback(std::bind(&Chatserver::onconnection,this,_1));
_server.setMessageCallback(bind(&Chatserver::onMessage,this,_1,_2,_3));
_server.setThreadNum(4);
}
//通信事务
void Chatserver::onMessage(const TcpConnectionPtr&conn,Buffer*buffer,Timestamp time)
{
string buf=buffer->retrieveAllAsString();
json js=json::parse(buf);//反序列化
//创建业务单例对象，根据json得到的msgid，运行对应的业务函数
Chatservice::instance()->gethandler(js["msgid"].get<int>())(conn,js,time);


}
//连接事务
void Chatserver::onconnection(const TcpConnectionPtr &conn)
{
    std::cout<<"onconnection"<<std::endl;
if(!conn->connected())
{
    std::cout<<"client disruption"<<std::endl;
    //调用客户端异常断开连接处理函数
   Chatservice::instance()->clientCloseException(conn);
    //连接失败或者连接中断
    conn->shutdown();
}

}
//开启
void Chatserver::start()
{
    _server.start();
}
