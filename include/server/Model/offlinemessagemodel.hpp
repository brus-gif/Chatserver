//离线消息表映射业务模块
#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include<iostream>
#include<vector>
#include<string>
class OfflineMsgModel
{
public:
//用数据库存储用户的离线消息
void insert(int id,std::string msg);

//删除用户的所有离线消息
void remove(int userid);

//查询用户的所有离线消息
std::vector<std::string> query(int userid);

};




#endif