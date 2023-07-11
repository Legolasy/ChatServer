#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include<functional>
#include<unordered_map>
#include<muduo/net/TcpConnection.h>
#include "public.hpp"
#include "json.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
// 事件回调handler
using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp)>;

//业务类
class ChatService
{
public:
    //单例模式借口
    static ChatService* instance();
    //登陆业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp);
    //注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:
    ChatService(); //构造函数私有化
    unordered_map<int,MsgHandler> _msgHandlerMap;
    
};

#endif