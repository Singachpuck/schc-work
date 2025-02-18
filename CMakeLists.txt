cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 11)
#set(EXECUTABLE_NAME "schc_work")

set(APP_DIRECTORY "${CMAKE_SOURCE_DIR}/applications")
set(APP_NAME "demo")
#set(CMAKE_VERBOSE_MAKEFILE ON)

include(config.cmake)

add_subdirectory(thirdparty)

project(schc_work C)

#add_executable(${EXECUTABLE_NAME} "main.c" "server.c")

#target_include_directories(${EXECUTABLE_NAME} PRIVATE "./include")
#target_link_libraries(schc_work schc)
