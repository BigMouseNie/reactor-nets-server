add_library(eventhandler STATIC
    sockethandler.cpp
)

target_include_directories(eventhandler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
