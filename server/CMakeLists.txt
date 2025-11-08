cmake_minimum_required(VERSION 4.0)
project(Lab3 C)

set(CMAKE_C_STANDARD 11)

add_executable(Lab3 server.c
        accept.c
        accept.h
        io.c
        io.h
        registry.c
        registry.h
        router.c
        router.h
        heartbeat.c
        heartbeat.h
        log.c
        log.h
        util.c
        util.h)
