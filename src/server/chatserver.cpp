#include "chatserver.hpp"
#include<functional>
using namespace std;
using namespace placeholders;
ChatServer::ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg)
            :_server(loop,listenAddr,nameArg),_loop(loop)
{
    //注册 conn 回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConn,this,_1));

    //注册 读写 回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置 线程数量
    _server.setThreadNum(4);
}
//启动服务
void ChatServer::start()
{
    _server.start();
}

//连接回调
void ChatServer::onConn(const TcpConnectionPtr& )
{

}
//读写回调
void ChatServer::onMessage(const TcpConnectionPtr&,Buffer*,Timestamp)
{

}
