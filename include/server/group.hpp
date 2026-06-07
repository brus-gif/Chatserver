//数据库群组映射模块
#ifndef GROUP_H
#define GROUP_H
#include<iostream>
#include<string>
#include<vector>
#include"groupuser.hpp"
class Group
{
private:
std::vector<GroupUser>users;//该群组中的组员对象
    int groupid;//组id
std::string groupname;//群组名
std::string desc;    //群组描述
public:
//初始化群对象
    Group(int id=-1,std::string name="",std::string desc="")
    {
this->groupid=id;
this->groupname=name;
this->desc=desc;
    }
    //获得群组id
int getId(){return groupid;}
//获取群组名字
std::string getName(){return groupname;}
//获取群组的详情介绍
std::string getDesc(){return desc;}   
//获取该群组成员的列表
std::vector<GroupUser>& getUsers(){return users;}

//设置群组的id
void setId(int id){this->groupid=id;}
//设置群组的名字
void setName(std::string name){this->groupname=name;}
//设置群组的详情介绍
void setDesc(std::string desc){this->desc=desc;}

~Group(){}

};

#endif

