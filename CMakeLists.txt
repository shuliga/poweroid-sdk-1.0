cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(SDK_NAME poweroid-sdk-1.0)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_RULE_MESSAGES OFF)

message(STATUS "Building ${SDK_NAME} library from ${CMAKE_CURRENT_SOURCE_DIR}")

project(${SDK_NAME} CXX)

if(NOT BOARD)
    set(BOARD uno_pro)
    #set(BOARD pro)
endif()

include_directories(lib)
link_directories(lib)

file(GLOB_RECURSE HDRS src/*.h)
message(STATUS "SDK Headers added: ${HDRS}")

file(GLOB_RECURSE SRCS src/*.cpp)
message(STATUS "SDK Sources added: ${SRCS}")

file(GLOB_RECURSE LIB_HDRS lib/*.h)
message(STATUS "Lib Headers added: ${LIB_HDRS}")

generate_arduino_library(${BOARD}_${SDK_NAME} BOARD ${BOARD} SRCS ${SRCS} HDRS ${HDRS} ${LIB_HDRS})
