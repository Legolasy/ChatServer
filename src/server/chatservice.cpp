#include "chatservice.hpp"
#include<muduo/base/Logging.h>
#include<string>
#include "mypublic.hpp"
#include "redis.hpp"
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
        _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});

        //群组业务
        // 群组业务管理相关事件处理回调注册
        _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
        _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
        _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});


        // 连接 redis 服务器
        if (_redis.connect())
        {
            // 设置上报消息的回调函数
            _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
        }

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

                // user登陆成功 向redis 订阅channel(id)
                _redis.subscribe(user.getId());

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

                // 查询好友信息 并返回
                vector<User> userVec = _friendModel.query(user.getId());
                if(!userVec.empty())
                {
                    LOG_INFO<<"User Have friends!";
                    // 返回好友信息
                    vector<string> vec2;
                    for(User &user : userVec)
                    {
                        json tmp;
                        tmp["id"] = user.getId();
                        tmp["name"] =  user.getName();
                        tmp["state"] = user.getState();
                        vec2.emplace_back(tmp.dump());
                    }
                    js_response["friends"] = vec2;
                }

                // 查询用户的群组信息
                vector<Group> groupuserVec = _groupModel.queryGroups(id);
                if (!groupuserVec.empty())
                {
                    // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                    vector<string> groupV;
                    for (Group &group : groupuserVec)
                    {
                        json grpjson;
                        grpjson["id"] = group.getId();
                        grpjson["groupname"] = group.getName();
                        grpjson["groupdesc"] = group.getDesc();
                        vector<string> userV;
                        for (GroupUser &user : group.getUsers())
                        {
                            json js;
                            js["id"] = user.getId();
                            js["name"] = user.getName();
                            js["state"] = user.getState();
                            js["role"] = user.getRole();
                            userV.push_back(js.dump());
                        }
                        grpjson["users"] = userV;
                        groupV.push_back(grpjson.dump());
                    }

                    js_response["groups"] = groupV;
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
    // 处理注销业务
    void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
    {
        int userid = js["id"].get<int>();
        {
            lock_guard<mutex> lock(_connMutex);
            auto it = _userConnMap.find(userid);
            if (it != _userConnMap.end())
            {
                _userConnMap.erase(it);
            }
        }
        // 用户注销，相当于就是下线，在redis中取消订阅通道
        _redis.unsubscribe(userid); 
        // 更新用户的状态信息
        User user(userid, "", "", "offline");
        _userModel.updateState(user);
    }
    // 一对一聊天
    void ChatService::oneChat(const TcpConnectionPtr &conn, json &js,Timestamp time) {
        
        int toid = js["toid"].get<int>();
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
        //toid 不在线 判断是否在其他机器上登陆
        User user = _userModel.query(toid);
        if(user.getState()=="online")
        {
            //在线 发到redis
            _redis.publish(toid,js.dump());
            return;
        }
        else
        {
            //不在线 离线转存
            _offlineMsgModel.insert(toid,js.dump());
        }
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
    //服务器退出，重置用户state
    void ChatService::reset() {
        //重置用户state to offline
        _userModel.resetState();
    }
    //添加好友业务
    void ChatService::addFriend(const TcpConnectionPtr &conn, json &js,Timestamp time) {
        int uid = js["id"].get<int>();
        int fid = js["friendid"].get<int>();

        //验证fid是否存在 todo

        //存储好友信息
        _friendModel.insert(uid,fid);

    }

    // 创建群组业务
    void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
    {
        int userid = js["id"].get<int>();
        string name = js["groupname"];
        string desc = js["groupdesc"];

        // 存储新创建的群组信息
        Group group(-1, name, desc);

        //group id 由 数据库主键自动生成
        if (_groupModel.createGroup(group))
        {
            // 把创建者 加入群
            _groupModel.addGroup(userid, group.getId(), "creator"); // creator 改成const常量 todo
        }
    }

    // 加入群组业务
    // 添加客户端响应 todo
    void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
    {
        int userid = js["id"].get<int>();
        int groupid = js["groupid"].get<int>();
        _groupModel.addGroup(userid, groupid, "normal");
    }

    // 群组聊天业务
    void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
    {
        int userid = js["id"].get<int>();
        int groupid = js["groupid"].get<int>();
        // 群用户id
        vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
        lock_guard<mutex> lock(_connMutex);
        for (int id : useridVec)
        {
            //群用户id对应的conn
            auto it = _userConnMap.find(id); 
            //在线
            if (it != _userConnMap.end())
            {
                // 转发群消息
                it->second->send(js.dump());
            }
            else
            {
                // //不在线
                // _offlineMsgModel.insert(id,js.dump());

                // 查询toid是否在线 
                User user = _userModel.query(id);
                if (user.getState() == "online")
                {
                    _redis.publish(id, js.dump());
                }
                else
                {
                    // 存储离线群消息
                    _offlineMsgModel.insert(id, js.dump());
                }
            }
        }
    }

    // 从redis消息队列中获取订阅的消息
    void ChatService::handleRedisSubscribeMessage(int userid, string msg)
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            it->second->send(msg);
            return;
        }

        // 存储该用户的离线消息
        _offlineMsgModel.insert(userid, msg);
    }