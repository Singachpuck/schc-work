cmake_minimum_required(VERSION 3.21)

include(config.cmake)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AddGitSubmodule)

set(SCHC_DIR "schc/full-sdk-delivery")
set(DELIVERY_DIR "${CMAKE_BINARY_DIR}/${TOOLCHAIN}/${TARGET}/deliver")
add_git_submodule(${SCHC_DIR} FALSE)
include(${CMAKE_CURRENT_SOURCE_DIR}/${SCHC_DIR}/full-sdk/toolchains/${TOOLCHAIN}/${TARGET}.cmake)

project(schc_work C)

# Sets APP_DIRECTORY and EXEC_NAMES
add_subdirectory(applications)

add_subdirectory(configure_files)

add_subdirectory(libs)

add_subdirectory(${SCHC_DIR})
