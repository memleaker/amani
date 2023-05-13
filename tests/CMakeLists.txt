project(tests)

# cpp standard
set(CMAKE_CXX_STANDARD 20)

# headers
include_directories(../include)

# compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")

# compiler flag
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-g -Wall -fcoroutines")

# target
add_executable(thpool-test thpool-test.cpp)
target_link_libraries(thpool-test pthread)

add_executable(copool-echoserver copool-echoserver.cpp)
target_link_libraries(copool-echoserver pthread)

add_executable(http-test http-test.cpp ../src/http.cpp)