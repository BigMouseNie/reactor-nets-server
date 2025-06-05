# 添加子模块
add_subdirectory(demultiplexer)
add_subdirectory(acceptor)

# 聚合为一个 netcore 静态库
add_library(netcore INTERFACE)

# 把两个模块链接到 netcore
target_link_libraries(netcore
    INTERFACE
    demultiplexer
    acceptor
)
