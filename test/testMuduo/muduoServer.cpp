#include <muduo/net/TcpServer.h> // for server
#include<iostream>
#include <muduo/net/EventLoop.h> // for client
#include<functional>
using namespace std;
using namespace muduo::net;
using namespace muduo;
using namespace placeholders;
//网络IO 业务 模块结耦
/*
1.TCPserver 
2.EventLoop 事件循环对象
3.构造ChatServer
4.注册回调函数，连接、读写事件
5.设置线程数量，Muduo 会自动分配IO线程、工作线程数量
*/
class ChatServer
{

public:
    ChatServer(EventLoop* loop, // 事件循环 epoll
            const InetAddress& listenAddr, // IP+Port
            const string& nameArg) // 服务器线程名称
            :_server(loop,listenAddr,nameArg),_loop(loop)
            {
               //1.注册回调 -> 用户连接事件
              _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
               //2.注册回调 -> 用户读写事件
              _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3)) ;
               //3.设置线程数量
               _server.setThreadNum(4);  // 1 IO + 3 workers 
            } 
    void start(){
      _server.start();
    }
private:
    //用户连接事件的回调函数
    void onConnection(const TcpConnectionPtr &conn)
    {
      if(conn->connected())
      {
        cout<<conn->peerAddress().toIpPort()<<" -> "<<
          conn->localAddress().toIpPort()<<" state::on"<<endl;
      }
      else
      {
        cout<<conn->peerAddress().toIpPort()<<" -> "<<
          conn->localAddress().toIpPort()<<" state::off"<<endl;

        conn->shutdown(); //close(fd)
      }
      }
      //用户读写事件的回调函数
    void onMessage(const TcpConnectionPtr &conn,Buffer *buffer,Timestamp time)
    {
      string buf = buffer->retrieveAllAsString();
      cout<<"recv:"<<buf<<" time:"<<time.toString()<<endl;
      conn->send(buf);
    };
    TcpServer _server;
    EventLoop *_loop; // epoll
  };

  int main()
  { 
    EventLoop loop; //epoll
    InetAddress addr("127.0.0.1",6666);//IP+PORT
    ChatServer server(&loop,addr,"ChatServer");

    server.start();//注册epoll
    loop.loop(); //epoll_wait 阻塞 监听事件

    return 0;
  }