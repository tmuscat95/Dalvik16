cmake_minimum_required(VERSION 3.9)
project(Dalvik16_Wrapper C)

set(CMAKE_C_STANDARD 99)

add_executable(Dalvik16_Wrapper main.c interpreter.c interpreter.h definitions.h loader.c loader.h)