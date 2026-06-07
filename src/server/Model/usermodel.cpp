// 数据库映射模块
#include "usermodel.hpp"
#include "db.hpp"
#include <iostream>

// 将对应的用户信息插入数据库的User表中
bool Usermodel::insert(User &user)
{
    char sql[1024] = {0};
    // User表中的id是自增主键，不需要插入对对应的数据，state只与登录有关，未登录，直接沿用之前的默认offline状态
    sprintf(sql, "insert into User (name,password,state) values ('%s','%s','%s')", user.getName().c_str(),
            user.getPwd().c_str(), user.getState().c_str()); // 执行对应的sql语句
    MySQL mysql;                                             // 创建临时mysql实例
    if (mysql.connect())                                     // 判断是否能建立连接
    {
        std::cout << "error" << std::endl;
        if (mysql.update(sql))
        {
            std::cout << "插入成功，直接返回" << std::endl;
            // 插入成功，直接返回
            user.setId(mysql_insert_id(mysql.getConnection())); // 获取自增的主键会涉及竟态条件，加锁
            return true;
        }
    }
    std::cout << "无法建立连接或插入失败" << std::endl;
    return false; // 无法建立连接，插入失败
}

// 查询用户的信息
User Usermodel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id=%d", id);

    MySQL mysql;
    if (mysql.connect())
    {
        // 建立了连接
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User(); // 没建立连接，直接返回默认初始化user
}

// 更新用户在数据库中记录的状态state（可以是online，也可以是offline）
bool Usermodel::updatestate(User &user)
{
    std::cout << "enter updatestate userstate: " << user.getState() << std::endl;
    char sql[1024] = {0};
    sprintf(sql, "update User set state='%s' where id=%d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect()) // 成功连接数据库
    {
        if (mysql.update(sql)) // 修改成功
        {
            return true;
        }
    }
    return false;
}

void Usermodel::resetState()
{
    char sql[1024] = {0};
    MySQL mysql;
    sprintf(sql, "update User set state='offline' where state='online'");
    if (mysql.connect())
    {
        // 连接成功
        mysql.update(sql);
    }
}