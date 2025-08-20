cmake_minimum_required(VERSION 3.22.1)

project(calc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    calc
    SHARED
    native-lib.cpp
    calc.cpp
    parsing.cpp
    evaluator.cpp
)

find_library(
    log-lib
    log
)

target_include_directories(calc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    calc
    ${log-lib}
    log
)

