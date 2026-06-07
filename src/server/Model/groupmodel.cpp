// 群组数据操作类实现

#include "groupmodel.hpp"
#include "db.hpp"

// 创建群组
bool GroupModel::CreateGroup(Group &group) // 引用传递，方便获取id（不需要返回值）
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup (groupname,groupdesc) values('%s','%s') ", group.getName().c_str(),
            group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {

        if (mysql.update(sql))
        {
            // 如果创建成功，返回获得群组id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, std::string role)
{
    char sql[1024] = {0};
    MySQL mysql;
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s')", groupid, userid, role.c_str());
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在的群组和群组中的群成员信息
std::vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    // 先查用户加入的群组有哪些
    sprintf(sql,
            "select a.groupid,b.groupname,b.groupdesc from GroupUser AS a join AllGroup AS b on a.groupid=b.id where "
            "a.userid=%d",
            userid);
    MySQL mysql;
    std::vector<Group> groupVec; // 创建一个vector来存储查询到的群组有哪些
    if (mysql.connect())
    {
        // 查询用户加入的群组
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            // 把查到的群组取出
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);

        // 查询用户加入的群组中的各个用户的信息，共用一个MySQL连接
        // 一次性查到的只能是跟当前用户有共同组的用户，所以需要根据每个群组来查询一次
        for (Group &group : groupVec) // 对group的修改会反映到groupVec中
        {
            memset(sql, 0, strlen(sql));
            sprintf(sql,
                    "select a.id,a.name,a.state,b.grouprole from User AS a join GroupUser AS b on a.id=b.userid where "
                    "b.groupid=%d",
                    group.getId());
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res); // 必须释放，不可复用（res存储每次查询每个组的成员信息返回的结果集，
                // 所以必须在for中，所以每次循环都会创建res，所以每次必须释放）
            }
            return groupVec;
        }
    }
    return groupVec; // 组里没人返回空的vector
}

// 根据指定的groupid查询群组用户的id列表，主要用于群聊业务，发消息给群中除自己之外的成员
std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid=%d AND userid!=%d ", groupid,
            userid); // 不用查用户状态，因为外部的unordered_map存储了用户的连接，也就是用户的状态
    MySQL mysql;
    std::vector<int> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}
