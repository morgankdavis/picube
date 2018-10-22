
cmake_minimum_required(VERSION 3.0)

project(picube)

set(CMAKE_CXX_STANDARD 14)


if ((CMAKE_BUILD_TYPE STREQUAL "") OR (NOT DEFINED CMAKE_BUILD_TYPE))
	set(CMAKE_BUILD_TYPE "Debug")
endif ()

message("*** BUILD TYPE: " ${CMAKE_BUILD_TYPE} " ***")

if (UNIX AND APPLE)
    message("We're MACOS")
    set(MACOS TRUE)
    add_definitions("-DMACOS")
    add_definitions("-DGETTIMEOFDAY_TWO_ARGS") # for Xscreensaver's 'yarandom'
    if (CMAKE_GENERATOR STREQUAL "Xcode")
        set(XCODE TRUE)
        add_definitions("-DXCODE")
    endif()
endif()

if (UNIX AND NOT APPLE AND NOT ANDROID AND NOT MINGW)
    message("We're LINUX")
    set(LINUX TRUE)
    add_definitions("-DLINUX")
    add_definitions("-DGETTIMEOFDAY_TWO_ARGS") # for Xscreensaver's 'yarandom'
endif()



file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.c*)

include_directories(include)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})



list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")



# GLFW
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

message("things" ${GLFW_INCLUDE_DIRS})


# GLEW
set(GLEW_USE_STATIC_LIBS ON)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
add_definitions(${GLEW_DEFINITIONS})
	
# GLM
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# Assimp
#find_package(Assimp REQUIRED)
#include_directories(${ASSIMP_INCLUDE_DIRS})

# gif-h

include_directories(external/gif-h)

# xscreensaver-rotator

add_subdirectory(external/xscreensaver-rotator)
include_directories(external/xscreensaver-rotator/include)

#led-matrix

if (LINUX)
	add_subdirectory(external/led-matrix)
	include_directories(external/led-matrix/include)
	target_link_libraries(${PROJECT_NAME} led-matrix)
endif()



target_link_libraries(${PROJECT_NAME}
	${GLFW_LIBRARIES}
	${GLEW_LIBRARIES}
	${GLM_LIBRARIES}
	xscreensaver-rotator)
