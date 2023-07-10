cmake_minimum_required(VERSION 3.0)
project(main)  #工程名称main

# 配置编译选项
set(CMAKE_CXX_FlAGS ${CMALE_CXX_FLAGS} -g)

# 头文件搜索路径
# include_directories()
# 库文件搜索路径
# link_directories()

#设置需要编译的源文件
set(SRC_LIST ./muduoServer.cpp)

# 设置可执行文件最终存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 生成可执行文件server 由 SRC_LIST 定义的源文件编译而来
add_executable(server ${SRC_LIST})

#链接库文件
target_link_libraries(server muduo_net muduo_base pthread)