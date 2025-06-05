add_library(threadpool STATIC
    threadpool.cpp
)

target_include_directories(threadpool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
