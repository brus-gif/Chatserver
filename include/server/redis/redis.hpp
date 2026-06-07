// redis公共调用模块
#ifndef REDIS_H
#define REDIS_H
#include <errno.h>
#include <functional>
#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <thread>
class Redis
{
  public:
    Redis();
    ~Redis();
    // 连接redis服务器
    bool connect();
    // 向redis指定的通道订阅
    bool subscribe(int channel);
    // 向redis指定的通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);

    // 在独立的接收线程接收订阅通道中的消息
    void observer_channel_message();

    // 初始化回调函数
    void init_notify_handler(std::function<void(int, std::string)> fn);

    // 向redis指定的通道channel发布消息
    bool publish(int channel, std::string message);

  private:
    redisContext *publish_contex;                                  // 负责pubilsh消息
    redisContext *subcribe_contex;                                 // 负责subscribe消息
    std::function<void(int, std::string)> _notify_message_handler; // 根据对应的订阅调用对对应的回调函数
};

#endif