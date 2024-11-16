cmake_minimum_required(VERSION 3.10)
project(MMT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(OpenCV_DIR "E:/vcpkg/installed/arm64-windows/share/opencv")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
    main.cpp
    server_socket.cpp
    client_socket.cpp
)

add_executable(client ${SOURCES})

target_link_libraries(client ${OpenCV_LIBS} ws2_32 gdi32 gdiplus ole32)
