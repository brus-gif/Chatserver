//数据库好友表映射模块
#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include<vector>
#include<string>
#include"db.hpp"
#include"user.hpp"
class FriendModel
{
    public:
    //添加新好友
void insert(int userid,int friendid);


//查询好友列表
std::vector<User> query(int userid);

};

#endif