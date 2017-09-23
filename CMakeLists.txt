cmake_minimum_required(VERSION 3.8)
project(lab2)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c command.c)
add_executable(lab2 ${SOURCE_FILES})