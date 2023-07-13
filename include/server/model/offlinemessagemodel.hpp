#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include<vector>
#include<string>
using namespace std;
//提供离线消息表 接口
class OfflineMsgModel
{
public:
    //添加离线消息
    void insert(int uid,string msg);
    //删除用户的离线消息 
    void remove(int uid);
    //查询
    vector<string> query(int uid);
};

#endif