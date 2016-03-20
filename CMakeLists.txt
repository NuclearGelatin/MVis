cmake_minimum_required(VERSION 3.3)
project(MVis)

#find_package(Freetype REQUIRED)
#include_directories(${FREETYPE_INCLUDE_DIRS})
#link_libraries(${FREETYPE_LIBRARIES})

#set(VTK_USE_SYSTEM_FREETYPE "ON")

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
link_libraries(${VTK_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp MathFunctions.hpp)
add_executable(MVis ${SOURCE_FILES})


