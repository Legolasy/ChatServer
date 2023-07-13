#include "usermodel.hpp"
#include "db.h"
#include<iostream>
#include<muduo/base/Logging.h>
bool UserModel::Insert(User &user)
{
    //1.组装sql
    char sql[1024]={0};
    sprintf(sql,"insert into user(name, password, state) values('%s','%s','%s')",
        user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());
    
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入user生成的主键作为 user的id
            user.setId(mysql_insert_id(mysql.getConn()));
            LOG_INFO<<"INSERT USER SUCCESS";
            return true;
        }
    }

    LOG_INFO<<"INSERT USER FAILED";
    return false;
}

User UserModel::query(int id)
{
    //1.组装sql
    char sql[1024]={0};
    sprintf(sql,"select * from user where id = %d",id);
    
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
       MYSQL_RES *res = mysql.query(sql);
       if(res!=nullptr)
       {
            auto row = mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                //释放动态资源
                mysql_free_result(res);
                return user;
            }
       }
    }
    return User();
}
bool UserModel::updateState(User user)
{
    //1.组装sql
    char sql[1024]={0};
    
    sprintf(sql,"update user set state = '%s' where id = %d",user.getState().c_str(),user.getId());
    
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
       if (mysql.update(sql))
            return true;
    }
    return false;
}

void UserModel::resetState() {
     //1.组装sql
    char sql[1024]="update user set state = 'offline' ";
    
    //sprintf(sql,"update user set state = 'offline' ");
    
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
       mysql.update(sql);
    }
}
