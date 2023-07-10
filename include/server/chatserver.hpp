#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;


//聊天服务器主类
class ChatServer
{
public:
// 初始化聊天服务器对象
ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
           
//启动服务
void start();

private:
    TcpServer _server; //muduo库 实现服务器功能的类对象
    EventLoop* _loop;  //事件循环对象 指针

    //连接事件回调
    void onConn(const TcpConnectionPtr&);
    //读写事件回调
    void onMessage(const TcpConnectionPtr&,Buffer*,Timestamp);
};

#endif