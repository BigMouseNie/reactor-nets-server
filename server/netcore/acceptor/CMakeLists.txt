add_library(acceptor STATIC
    acceptor.cpp
)

target_include_directories(acceptor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
