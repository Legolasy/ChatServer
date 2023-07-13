#include "friendmodel.hpp"
#include "user.hpp"
#include "db.h"
#include <muduo/base/Logging.h>
void FriendModel::insert(int uid, int fid) {
    // 1.组装sql
  char sql[1024] = {0};
  sprintf(sql, "insert into friend values(%d,%d)",uid,fid);
  // 2.连接Mysql
  MySQL mysql;
  if (mysql.connect()) {
    mysql.update(sql);
  }
}
vector<User> FriendModel::query(int uid) {
  LOG_INFO<<"Query Friend Start";
  //1.组装sql
    char sql[1024]={0};
    //联合查询，uid的好友，返回好友的name state id
    sprintf(sql,"select a.id,a.name,a.state from user a inner join friend b on b.friendid=a.id where b.userid=%d",uid);
    vector<User>vec;
    //2.连接Mysql
    MySQL mysql;
    if(mysql.connect())
    {
       MYSQL_RES *res = mysql.query(sql);
       if(res!=nullptr)
       {
            LOG_INFO<<"Query Friend Find!!!";
            MYSQL_ROW row ;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.emplace_back(user);
            }
            //释放动态资源
            mysql_free_result(res);
            return vec;
       }
    }
    return vec;
}