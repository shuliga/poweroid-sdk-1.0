cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(SDK_NAME Poweroid_SDK_10)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_RULE_MESSAGES OFF)

message(STATUS "Building ${SDK_NAME} library from ${CMAKE_CURRENT_SOURCE_DIR}")

project(${SDK_NAME} CXX)

if(NOT BOARD)
    #set(BOARD uno_pro)
    set(BOARD pro)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

file(GLOB_RECURSE HDRS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
message(STATUS "SDK Headers added: ${HDRS}")

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
message(STATUS "SDK Sources added: ${SRCS}")

file(GLOB_RECURSE LIB_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h)
message(STATUS "Lib Headers added: ${LIB_HDRS}")

generate_arduino_library(${BOARD}_${SDK_NAME} BOARD ${BOARD} SRCS ${SRCS} HDRS ${HDRS} ${LIB_HDRS})
