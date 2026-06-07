#include "redis.hpp"
Redis::Redis() : publish_contex(nullptr), subcribe_contex(nullptr)
{
}
Redis::~Redis()
{
    if (publish_contex != nullptr)
    {
        redisFree(publish_contex);
    }
    if (subcribe_contex != nullptr)
    {
        redisFree(subcribe_contex);
    }
}
// 连接redis服务器
bool Redis::connect()
{
    std::cout << "success build redis connection!" << std::endl;
    publish_contex = redisConnect("127.0.0.1", 6379);
    if (publish_contex == nullptr)
    {
        std::cerr << "there is no more space to give" << std::endl;
        return false;
    }
    else if (publish_contex->err != 0)
    {
        std::cerr << "publish redisconnect error!" << std::endl;
        return false;
    }
    subcribe_contex = redisConnect("127.0.0.1", 6379);
    if (subcribe_contex == nullptr)
    {
        std::cerr << "there is no more space to give" << std::endl;
        return false;
    }
    else if (subcribe_contex->err != 0)
    {
        std::cerr << "publish redisconnect error!" << std::endl;
        return false;
    }

    std::thread t([&]() { observer_channel_message(); });
    t.detach();
    std::cout << "the redis observer_channel_message start!" << std::endl;
    return true;
}
// 向指定的订阅发布消息
bool Redis::publish(int channel, std::string message)
{
    std::cout << "success publish the message:" << message << std::endl;
    redisReply *reply = (redisReply *)redisCommand(publish_contex, "publish %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        std::cerr << "publish redisCommand fail!" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道发消息
bool Redis::subscribe(int channel)
{
    std::cout << "enter subscribe channel!" << std::endl;
    if (REDIS_ERR == redisAppendCommand(this->subcribe_contex, "subscribe %d", channel))
    {
        std::cerr << "subscribe command failed!" << std::endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->subcribe_contex, &done))
        {
            std::cerr << "subscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}
// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->subcribe_contex, "unsubscribe %d", channel))
    {
        std::cerr << "subscribe command failed" << std::endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->subcribe_contex, &done))
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

// 在独立的接收线程接收订阅通道中的消息
void Redis::observer_channel_message()
{
    std::cout << "observer_channel_message start recive message!" << std::endl;
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->subcribe_contex, (void **)&reply))
    {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            std::cout << "success recive observer_channel_message!" << std::endl;
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
}

// 初始化回调函数
void Redis::init_notify_handler(std::function<void(int, std::string)> fn)
{
    _notify_message_handler = fn;
}
