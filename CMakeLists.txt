cmake_minimum_required(VERSION 3.0)

# test 需要放在头部
add_subdirectory(${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests)

# project
project(amani)

# cpp standard
set(CMAKE_CXX_STANDARD 20)

# headers
include_directories(include)

# source
file(GLOB SOURCES "src/*")

# compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")

# compiler flag
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-g -Wall -fcoroutines")

# traget
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
add_executable(amani ${SOURCES})

# ld
target_link_libraries(amani pthread ssl crypto)

# install
install(FILES amani DESTINATION /usr/bin/amani)

