add_library(demultiplexer STATIC
    epolldemux.cpp
)

target_include_directories(demultiplexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
