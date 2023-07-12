#include "chatservice.hpp"
#include<muduo/base/Logging.h>
#include<string>
#include "mypublic.hpp"
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
    void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp)
    {
        //LOG_INFO<<"Login Service!!!";
        int id = js["id"].get<int>();
        string pwd = js["password"];

        //比对数据库中的 账号密码
        User user = _userModel.query(id);
        if(user.getId()!=-1 &&user.getPwd()==pwd)
        {
            //匹配成功
            //是否已经登陆
            if(user.getState()=="online")
            {
                //已经登陆 不能重复登录
                json js_response;
                js_response["msgid"]=LOGIN_MSG_ACK;
                js_response["errno"] = 2; //错误类型 标识 不同错误
                js_response["errmsg"] = "LOGIN FAILED, Already Login!"; //errmsg 错误信息
                //js_response["id"]=user.getId();
                conn->send(js_response.dump());
            }
            else
            {
                //登陆ok
                //更新用户state off->online
                user.setState("online");
                _userModel.updateState(user);
                json js_response;
                js_response["msgid"]=LOGIN_MSG_ACK;
                js_response["errno"] = 0; //错误类型 标识 0代表没有错误
                js_response["id"]=user.getId();
                js_response["name"]=user.getName();
                conn->send(js_response.dump());
            }
        }
        else
        {
            //失败
            json js_response;
            js_response["msgid"]=LOGIN_MSG_ACK;
            js_response["errno"] = 1; //错误类型 标识 1代表有错误
            js_response["errmsg"] = "LOGIN FAILED,Wrong Account or PWD !"; //errmsg 错误信息
            //js_response["id"]=user.getId();
            conn->send(js_response.dump());
            
        }
    }
    //注册业务
    void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp)
    {
        //LOG_INFO<<"Reg Service!!!";
        string name = js["name"];
        string pwd = js["password"];

        User user;
        user.setName(name);
        user.setPwd(pwd);
        bool state = _userModel.Insert(user);

        if(state){
            //注册ok
            json js_response;
            js_response["msgid"]=REG_MSG_ACK;
            js_response["errno"] = 0; //错误类型 标识 0代表没有错误
            js_response["id"]=user.getId();
            conn->send(js_response.dump());
        }
        else{
            //注册failed
            json js_response;
            js_response["msgid"]=REG_MSG_ACK;
            js_response["errno"] = 1; //错误类型 标识 1代表有错误
            js_response["errmsg"] = "REG FAILED"; //errmsg 错误信息
            //js_response["id"]=user.getId();
            conn->send(js_response.dump());
        }
    }