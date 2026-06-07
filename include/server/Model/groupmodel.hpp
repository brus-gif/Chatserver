//数据库群组表事务处理模块
#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include"groupuser.hpp"
#include"group.hpp"
#include<vector>
class GroupModel
{
    public:
    GroupModel(){}
    ~GroupModel(){}
//创建群组
    bool CreateGroup(Group&group);
    //加入群组
    void addGroup(int userid,int groupid,std::string role);
    //查询用户所在的群组和群组中的群成员信息
std::vector<Group> queryGroups(int userid);
    //根据指定的groupid查询群组用户的id列表，主要用于群聊业务，发消息给群中的成员
std::vector<int> queryGroupUsers(int userid,int groupid);
};
        
#endif