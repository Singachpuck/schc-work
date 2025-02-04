cmake_minimum_required(VERSION 3.27)
project(schc_work C)

set(CMAKE_C_STANDARD 11)
set(EXECUTABLE_NAME "schc_work")

add_subdirectory(thirdparty)

add_executable(${EXECUTABLE_NAME} "main.c" "server.c")

target_include_directories(${EXECUTABLE_NAME} PRIVATE "./include")
#target_link_libraries(schc_work schc)
