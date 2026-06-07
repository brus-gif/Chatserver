#include "offlinemessagemodel.hpp"
#include "db.hpp"
// 用数据库存储用户的离线消息
void OfflineMsgModel::insert(int id, std::string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d,'%s')", id, msg.c_str());
    MySQL mysql;
    if (mysql.connect()) // 数据库连接成功成功
    {
        mysql.update(sql);
        return;
    }
}

// 删除用户的所有离线消息
void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);
    MySQL mysql;
    if (mysql.connect()) // 数据库连接成功
    {
        mysql.update(sql);
        return;
    }
}

// 查询用户的所有离线消息(只查消息本身，不查消息的相关信息，因为消息来源都写在json序列中)
std::vector<std::string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);
    MySQL mysql;
    std::vector<std::string> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) // 知道读到nullptr，把所有的离线消息都读完
            {
                vec.push_back(row[0]);
            }
            // 释放结果集res
            mysql_free_result(res);
        }
    }
    return vec;
}
