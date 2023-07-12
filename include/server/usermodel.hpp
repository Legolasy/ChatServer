#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"
//User表的数据操作类
class UserModel {
public:
    //User表 增加方法
    bool Insert(User &user);
    // 主键 查询 user
    User query(int id);
    //更新用户状态
    bool updateState(User user);
};
#endif