cmake_minimum_required(VERSION 3.10)
project(LoadBalancer CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)
add_executable(load_balancer
    src/main.cpp
    src/loadbalancer.cpp
    src/server.cpp
    src/healthchecker.cpp
    src/consistenthash.cpp
    src/stats.cpp
)
target_compile_options(load_balancer PRIVATE -Wall -Wextra -O2)
target_link_libraries(load_balancer pthread)
