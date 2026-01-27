cmake_minimum_required(VERSION 3.14)
project(mwacstl)

add_library(mwacstl
    cstr.cpp
)

target_include_directories(mwacstl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
