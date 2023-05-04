cmake_minimum_required(VERSION 3.20)

# 绑定版本为 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARY_REQUIRED ON)
set(CMAKE_EXTENSIONS ON)
message("[hint]: the C++ standard current used : C++${CMAKE_CXX_STANDARD}")

project(connect_pool LANGUAGES CXX)


include_directories(${PROJECT_SOURCE_DIR}/lib_mysql_connect)
include_directories(${PROJECT_SOURCE_DIR}/lib_connection_pool)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib_mysql_connect)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib_connection_pool)

add_subdirectory(${PROJECT_SOURCE_DIR}/test)

message("[hint]: the cmake build is complete")