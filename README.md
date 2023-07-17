# ChatServer
使用muduo网络库打造分布式集群聊天服务器项目，nginx配置tcp负载均衡，RR负载算法实现平滑提高并发能力，基于发布-订阅机制的redis消息队列实现跨服务器通信功能。

# 依赖库
## Nginx 配置TCP负载均衡
注意：Nginx1.9版本后才支持TCP长连接负载均衡，且默认没有编译TCP负载均衡模块，编译时加上 --with-stream 激活模块。
配置文件在conf目录中，修改nginx.conf如下：  


## Redis
## 

# 编译方法
cd build  
rm -rf *  
cmake ..  
make  
