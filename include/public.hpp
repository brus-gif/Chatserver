// 存放事件的编号（事件回调函数对应的键）
#ifndef PUBLIC_H
#define PUBLIC_H
enum MSG
{
    // 客户端->服务器
    LOGIN_MSG = 1, // 登录事件
    REG_MSG,       // 注册事件
    LOGINOUT_MSG,  // 注销登陆事件

    // 服务器->客户端
    REG_MSG_ACK,      // 注册响应消息
    LOGIN_MSG_ACK,    // 登陆响应事件
    LOGINOUT_MSG_ACK, // 注销登陆事件响应

    // 主要业务
    ONE_CHAT_MSG,     // 一对一聊天事件
    ADD_FRIENDM_MSG,  // 添加好友事件
    CREATE_GROUP_MSG, // 创建群组事件
    ADD_GROUP_MSG,    // 加入聊天群组事件
    GROUP_CHAT_MSG    // 群聊天事件
};

#endif