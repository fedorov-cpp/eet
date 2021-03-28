set(SRC
        Protocol.cpp
        Device.cpp
        Slave.cpp
        Master.cpp)
add_library(eet STATIC ${SRC})
target_include_directories(eet PUBLIC ${CMAKE_CURRENT_LIST_DIR})