#include"db.hpp"
//初始化数据模块对象
MySQL::MySQL()
{
_conn=mysql_init(nullptr);
}


//析构数据模块对象
MySQL::~MySQL()
{
if(_conn!=nullptr)
{
    //关闭数据库连接句柄，释放资源
mysql_close(_conn);
}
}


//建立与数据库的连接
bool MySQL::connect()
{
MYSQL*p=mysql_real_connect(_conn,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0);
if(p==nullptr)
{ 
mysql_query(_conn,"set names gbk");//指针非空为真
}
return p;//空指针为假
}


//更新操作——对数据库表的数据的的增删改
bool MySQL::update(string sql)
{
if(mysql_query(_conn,sql.c_str()))
{
LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"更新失败";
return false;
}
return true;
}


//对数据库表的查询操作,用完要mysql_free_result
MYSQL_RES* MySQL::query(string sql)
{
if(mysql_query(_conn,sql.c_str()))
{
LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败！";
return nullptr;
}
//返回结果集，长流式读取，但是结果集不存在本地(数据在服务端，流式逐行拿，连接被占用、游标只能前进)
return mysql_use_result(_conn);
}

MYSQL*MySQL::getConnection()
{ 
return _conn;
}