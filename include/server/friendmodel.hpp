#ifndef FRIEND_H
#define FRIEND_H

#include<vector>
#include"user.hpp"

using namespace std;
// 操作friend表对象的接口
class FriendModel
{
public:
    // 添加好友
    void insert(int uid,int fid);
    // 返回用户好友列表 （实际应该在客户端存 不变性
    vector<User>query(int uid);
};

#endif