# GTEST
set(GTEST_INCLUDE_DIR /usr/local/include)
include_directories(${GTEST_INCLUDE_DIR})
set(GTEST_LIB_DIR /usr/local/lib)
link_directories(${GTEST_LIB_DIR})

# TESTS
enable_testing()

#set(TESTS
#        test_Devices.cpp
#        test_Protocol.cpp
#        test_Helpers.cpp)
#include_directories(eet)
#add_executable(tests ${TESTS})
#target_link_libraries(tests gtest gtest_main ${PROJECT_NAME})
#add_test(NAME tests COMMAND tests)
