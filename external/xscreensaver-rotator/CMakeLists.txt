cmake_minimum_required(VERSION 3.0)

project(xscreensaver-rotator)

file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.c*)

include_directories(include)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})

