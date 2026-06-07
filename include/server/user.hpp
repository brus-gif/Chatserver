//数据库映射模块——数据库user表在本地的映射
#ifndef USER_H
#define USER_H

#include<string>

class User
{

public:
//初始化用户
User(int id=-1,std::string name="",std::string pwd="",std::string state="offline")
{
this->id=id;
this->name=name;
this->password=pwd;
this->state=state;
}

//修改操作
void setId(int id)
{
    this->id=id;
}
void setName(std::string name)
{
this->name=name;
}
void setPwd(std::string pwd)
{
this->password=pwd;
}
void setState(std::string state)
{
this->state=state;
}

//查询操作，获得对应的属性
int getId(){return this->id;}
std::string getName(){return this->name;}
std::string getPwd(){return this->password;}
std::string getState(){return this->state;}
private:
int id;//用户id
std::string name;//用户名
std::string password;//用户密码
std::string state;//用户状态

};

#endif