cmake_minimum_required(VERSION 3.14)
project(mwacstl C)

add_library(mwacstl
    cstr.c
    allocator.c
)

target_include_directories(mwacstl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
