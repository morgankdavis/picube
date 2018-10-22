
cmake_minimum_required(VERSION 3.0)

project(led-matrix)

set(CMAKE_CXX_STANDARD 14)


file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.c*)

include_directories(include)


add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})


#-lpthread  -lrt -lm -lpthread

target_link_libraries(${PROJECT_NAME}
	pthread
	rt
	m)
