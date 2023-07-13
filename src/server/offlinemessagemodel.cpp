#include "offlinemessagemodel.hpp"
#include "db.h"
#include<mysql/mysql.h>
#include<muduo/base/Logging.h>
void OfflineMsgModel::insert(int uid, string msg) {
    //1.组装sql
    char sql[1024]={0};
    sprintf(sql,"insert into OfflineMessage values('%d','%s')",
    uid,msg.c_str());
    
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            LOG_INFO<<"INSERT Offline MSG SUCCESS";
            return;
        }
    }
}

void OfflineMsgModel::remove(int uid) {
    //1.组装sql
    char sql[1024]={0};
    sprintf(sql,"delete from OfflineMessage where userid=%d",
    uid);
    
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            LOG_INFO<<"DELETE Offline MSG SUCCESS";
            return;
        }
    }
}

vector<string> OfflineMsgModel::query(int uid) { 
    //1.组装sql
    char sql[1024]={0};
    sprintf(sql,"select message from OfflineMessage where userid=%d",
    uid);
    vector<string>vec;
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) !=nullptr)
            {
                vec.emplace_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
