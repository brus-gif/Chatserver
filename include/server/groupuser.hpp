//数据库群组用户映射模块
#ifndef GROUPUSER_H
#define GROUPUSER_H
#include"user.hpp"

class GroupUser:public User
{
private:
   std::string role;//用户在群组的身份（普通成员/管理员）
public:
//设置当前群用户在群组的身份
void setRole(std::string role){this->role=role;}
//查询当前群用户在群组中的身份
std::string getRole(){return role;}
    GroupUser(/* args */){}
    ~GroupUser(){}
};
#endif