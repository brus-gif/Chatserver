//数据库User表的映射模块的头文件
#ifndef USERMODEL_H
#define USERMODEL_H
#include"user.hpp"
class Usermodel
{
    public:
//将用户插入数据库的user表
    bool insert(User &user);

//查询用户的信息
User query(int id);

//更新用户在数据库中记录的状态state（可以是online，也可以是offline）
//使用user的id查询匹配
bool updatestate(User&user);

//重置数据库User表中所有用户的状态
void resetState();

};
#endif