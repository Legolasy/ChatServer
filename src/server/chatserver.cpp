#include "chatserver.hpp"
#include<functional>
#include "chatservice.hpp"
#include "json.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
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
void ChatServer::onConn(const TcpConnectionPtr& conn)
{
    // 断开
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
};
//读写回调
void ChatServer::onMessage(const TcpConnectionPtr& conn,Buffer* buffer,Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //json 反序列化
    json js = json::parse(buf);
    //解耦业务模块、网络模块 by 回调
    //js["msgid"] -> 业务handler 
    auto handler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    handler(conn,js,time);
};
