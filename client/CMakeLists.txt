cmake_minimum_required(VERSION 4.0)
project(Lab3 C)

set(CMAKE_C_STANDARD 11)

add_executable(Lab3 client.c
        cmd.c
        cmd.h
        ui.c
        ui.h)

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
