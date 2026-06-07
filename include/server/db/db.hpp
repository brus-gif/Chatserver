//数据模块，封装数据库原生函数

#ifndef DB_H
#define DB_H
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include<string>

using namespace std;
static string server="127.0.0.1";
static string user="root";
static string password="123456";
static string dbname="chat";

class MySQL
{
public:
//初始化数据模块
MySQL();

//RAII释放数据模块资源
~MySQL();

//建立与数据库的连接
bool connect();

//修改操作——对数据库表的数据的的增删改(这里的返回值只做是否成功的判断，要获得主键要获得连接句柄额外处理)
bool update(string sql);

//查询操作，返回查到的结果集（使用的是mysql_use_result，结果不在本地
//，所以查完后要用mysql_free_result把返回的结果集释放）
MYSQL_RES* query(string sql);


//获得当前数据库的连接句柄
MYSQL*getConnection();

private:
//数据库连接句柄
MYSQL*_conn;

};





#endif