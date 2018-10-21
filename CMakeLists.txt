
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
    if (CMAKE_GENERATOR STREQUAL "Xcode")
        set(XCODE TRUE)
    endif()
endif()

if (UNIX AND NOT APPLE AND NOT ANDROID AND NOT MINGW)
    message("We're LINUX")
    set(LINUX TRUE)
endif()



# GLFW

#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#add_subdirectory(external/glfw-3.2.1)

# GLEW

#add_subdirectory(external/glew-2.1.0)
#include_directories(external/glew-2.1.0/include)

# GLM

#include_directories(external/glm-0.9.9)


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




file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.c*)

include_directories(include)




add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})


target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${GLM_LIBRARIES})


#target_link_libraries(${PROJECT_NAME}
#	z
#	glfw
#	glew_s)



#if (MACOS)
#
#	find_library(FRAMEWORK_COCOA Cocoa)
#	find_library(FRAMEWORK_OPENGL OpenGL)
#	find_library(FRAMEWORK_IOKIT IOKit)
#	find_library(FRAMEWORK_CORE_VIDEO CoreVideo)
#	set(FRAMEWORKS ${FRAMEWORK_COCOA} ${FRAMEWORK_OPENGL} ${FRAMEWORK_IOKIT} ${FRAMEWORK_CORE_VIDEO})
#	target_link_libraries(${PROJECT_NAME} ${FRAMEWORKS})
#
#	set(CMAKE_MACOSX_RPATH 1) # silence "MACOSX_RPATH is not specified [...]"
#	
#elseif (LINUX)
#	
#	target_link_libraries(ae
#		GL
#		X11
#		Xxf86vm
#		Xrandr
#		pthread
#		Xi
#		dl)
#		
#endif()

