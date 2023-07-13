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
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
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
                LOG_INFO<<"LOGIN FAILED, Already Login!"; //errmsg 错误信息
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
                LOG_INFO<<"Login SUCCESS!";
                //记录用户登陆conn 注意多线程并发加锁
                {
                    LOG_INFO<<"Save User Conn!";
                    //析构自动释放锁 限制作用域
                    lock_guard<mutex>lock(_connMutex);
                    _userConnMap.insert({id,conn}); 
                }
                //更新用户state off->online
                LOG_INFO<<"User State Update off to online!";
                user.setState("online");
                _userModel.updateState(user);
                json js_response;
                js_response["msgid"]=LOGIN_MSG_ACK;
                js_response["errno"] = 0; //错误类型 标识 0代表没有错误
                js_response["id"]=user.getId();
                js_response["name"]=user.getName();
                
                //查询是否有离线消息
                auto vec = _offlineMsgModel.query(user.getId());
                if(!vec.empty())
                {
                    js_response["offlinemsg"]=vec; //json 适配容器
                    //读取后 删除用户的离线消息
                    _offlineMsgModel.remove(user.getId());
                }

                conn->send(js_response.dump());
            }
        }
        else
        {
            //失败
            LOG_INFO<<"LOGIN FAILED,Wrong Account or PWD !";
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

    // 一对一聊天
    void ChatService::oneChat(const TcpConnectionPtr &conn, json &js,Timestamp time) {
        
        int toid = js["to"].get<int>();
        {
            lock_guard<mutex>lock(_connMutex);
            //找到转发的id对象的conn 转发js
            auto it = _userConnMap.find(toid);
            if(it!=_userConnMap.end())
            {
                //找到了目标id 服务器把消息推给toid的conn
                it->second->send(js.dump());
                return;
            }
        }
        //toid 不在线 离线存储信息
        _offlineMsgModel.insert(toid,js.dump());
    }

    // 处理客户端异常退出
    void ChatService::clientCloseException(const TcpConnectionPtr &conn)
    {
        User user;
        {
            lock_guard<mutex>lock(_connMutex);
            //1._userConnMap 删除连接
            for(auto it=_userConnMap.begin();it!=_userConnMap.end();it++)
            {
                if(it->second == conn)
                {
                    user.setId(it->first);
                    _userConnMap.erase(it);
                    break;
                }
            }
        }
        if(user.getId() != -1)
        {
            LOG_INFO<<"User State Update on to offline!";
            user.setState("offline");
            _userModel.updateState(user); //mysql  保证线程安全
        }
    }