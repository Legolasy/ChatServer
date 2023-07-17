# ChatServer
使用muduo网络库打造分布式集群聊天服务器项目，nginx配置tcp负载均衡，RR负载算法实现平滑提高并发能力，基于发布-订阅机制的redis消息队列实现跨服务器通信功能。

# 编译方法
cd build  
rm -rf *  
cmake ..  
make  
