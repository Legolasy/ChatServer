#ifndef MYPUBLIC_H
#define MYPUBLIC_H

enum EnMsgType
{
    LOGIN_MSG = 1, //登陆消息id
    LOGIN_MSG_ACK,//登陆响应
    REG_MSG, //注册消息id
    REG_MSG_ACK, //注册响应消息
    ONE_CHAT_MSG, //点对点聊天消息
    ADD_FRIEND_MSG,//添加好友消息
    
    CREATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG, // 加入群组
    GROUP_CHAT_MSG, // 群聊天
};

#endif