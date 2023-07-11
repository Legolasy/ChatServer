#include "chatservice.hpp"
#include<muduo/base/Logging.h>
#include<string>
#include "public.hpp"
using namespace muduo;
//获取单例对象
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

//在构造函数中 注册id-函数
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
}
//获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    //没有此方法 日志记录错误 返回默认处理器 避免异常导致程序over
    if(it == _msgHandlerMap.end())
    {
        //返回默认处理器
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp)
        {
            LOG_ERROR << "msgid: "<<msgid <<" cannot find handler";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}
//登陆业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp)
    {
        LOG_INFO<<"Login Service!!!";
    }
    //注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp)
    {
        LOG_INFO<<"Reg Service!!!";
    }