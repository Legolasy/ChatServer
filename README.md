# ChatServer
使用muduo网络库打造分布式集群聊天服务器项目，nginx配置tcp负载均衡，RR负载算法实现平滑提高并发能力，基于发布-订阅机制的redis消息队列实现跨服务器通信功能。

# 运行截图
![Image text](https://github.com/Legolasy/ChatServer/blob/master/png/server_start.jpg)
![Image text](https://github.com/Legolasy/ChatServer/blob/master/png/client_test.jpg)

多个Server运行在不同端口，而Client连接的都是127.0.0.1:8000端口，此为Nginx监听端口，由Nginx负责将连接分配给服务器，使用RR算法，因此两台服务器分别获得一个连接。 
测试两个不同服务器下的连接互相通信：  
每个user使用Redis订阅本userid的channel，发送消息时，在目标userid的channel上发布信息，对应channel的user就会接收到信息，实现跨服务器通信。  

# 依赖库
## Nginx 配置TCP负载均衡
注意：Nginx1.9版本后才支持TCP长连接负载均衡，且默认没有编译TCP负载均衡模块，编译时加上 --with-stream 激活模块。
配置文件在conf目录中，修改nginx.conf如下：  
![Image text](https://github.com/Legolasy/ChatServer/blob/master/png/nginx_conf.jpg)
启动nginx 监听 8000端口，后续client连接server走8000端口，然后由nginx分配给不同的服务器。
![Image text](https://github.com/Legolasy/ChatServer/blob/master/png/nginx_start.jpg)
## Redis
ubuntu 安装  
sudo apt-get install redis-server  
安装完成会自动启动redis服务  
![Image text](https://github.com/Legolasy/ChatServer/blob/master/png/redis_start.jpg)  

# 编译方法
cd build  
rm -rf *  
cmake ..  
make  
