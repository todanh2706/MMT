cmake_minimum_required(VERSION 3.10)
project(MMT)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(MMT main.cpp client_socket.cpp server_socket.cpp)
target_link_libraries(MMT ${OpenCV_LIBS})
