#ifndef MYPUBLIC_H
#define MYPUBLIC_H

enum EnMsgType
{
    LOGIN_MSG = 1, //登陆消息id
    LOGIN_MSG_ACK,//登陆响应
    REG_MSG, //注册消息id
    REG_MSG_ACK, //注册响应消息
    ONE_CHAT_MSG, //点对点聊天消息
};

#endif