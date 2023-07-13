#include "chatserver.hpp"
#include<signal.h>
#include <iostream>
#include "chatservice.hpp"
using namespace std;
//处理服务器 ctrl+c 重置用户状态（offline）
void resetHandler(int )
{
    ChatService::instance()->reset();
    exit(0);
}
int main()
{
    signal(SIGINT,resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1",6666); // IP+PORT
    ChatServer server(&loop,addr,"ChatServer");

    server.start();
    loop.loop();

}