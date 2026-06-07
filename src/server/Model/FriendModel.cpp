#include"friendmodel.hpp"

void FriendModel::insert(int userid,int friendid)
{
char sql[1024]={0};
sprintf(sql,"insert into Friend values(%d,%d)",userid,friendid);
MySQL mysql;
if(mysql.connect())
{
mysql.update(sql);
}

}


//查询好友列表
std::vector<User> FriendModel::query(int userid)
{
    std::vector<User>vec;
MySQL mysql;
    char sql[1024]={0};
sprintf(sql," select b.id,b.name,b.state from Friend as a,User as b where a.friendid=b.id AND a.userid=%d",userid);
if(mysql.connect())
{
MYSQL_RES*res=mysql.query(sql);
MYSQL_ROW row;
if(res!=nullptr)
{
while((row=mysql_fetch_row(res))!=nullptr)
{
User user;
user.setId(atoi(row[0]));
user.setName(row[1]);
user.setState(row[2]);
vec.push_back(user);
}
}
mysql_free_result(res);

}
return vec;
}
