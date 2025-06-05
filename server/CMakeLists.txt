cmake_minimum_required(VERSION 3.12)
project(ReactorServer VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O2")

# 添加子目录
add_subdirectory(eventhandler)
add_subdirectory(netcore)
add_subdirectory(threadpool)

add_executable(server main.cpp reactorimpl.cpp subreactor.cpp)

# link
target_link_libraries(server PRIVATE eventhandler netcore threadpool)
