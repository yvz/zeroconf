cmake_minimum_required(VERSION 2.8)
project(zeroconf)

include_directories(./src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(ZEROCONF_TEST_SOURCE_FILES
    src/zeroconf.hpp
    src/zeroconf-detail.hpp
    src/zeroconf-util.hpp
    test/main.cpp
    test/Test_Parse.cpp
    test/Test_WriteFqdn.cpp)

add_executable(zeroconf_test ${ZEROCONF_TEST_SOURCE_FILES})
target_link_libraries(zeroconf_test libgmock.a pthread)

set(ZEROCONF_BASIC_DEMO_SOURCE_FILES
    src/zeroconf.hpp
    src/zeroconf-detail.hpp
    src/zeroconf-util.hpp
    samples/basic_demo/main.cpp)

add_executable(basic_demo ${ZEROCONF_BASIC_DEMO_SOURCE_FILES})