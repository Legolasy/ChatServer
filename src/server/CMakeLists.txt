
# 变量SRC_LIST  代表目录下所有源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件
add_executable(ChatServer ${SRC_LIST})

# 链接时依赖的库
target_link_libraries(ChatServer muduo_net muduo_base pthread)

#配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)