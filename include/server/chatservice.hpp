#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include<functional>
#include<unordered_map>
#include<muduo/net/TcpConnection.h>
#include "mypublic.hpp"
#include "json.hpp"
#include "usermodel.hpp"
#include<mutex>
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
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
    //注销登录
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp);
    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常错误，业务重置
    void reset();
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // redis 上报消息 回调函数
    void handleRedisSubscribeMessage(int userid, string msg);

   private:
    ChatService(); //构造函数私有化

    //回调函数 映射map
    unordered_map<int,MsgHandler> _msgHandlerMap;
    //用户 id-conn 映射map
    unordered_map<int,TcpConnectionPtr> _userConnMap;    
    //互斥锁
    mutex _connMutex;
    //数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // redis 操作对象
    Redis _redis;
};

#endif