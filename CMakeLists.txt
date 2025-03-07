cmake_minimum_required(VERSION 3.21)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(AddGitSubmodule)
include(config.cmake)

# Sets APP_DIRECTORY and EXEC_NAMES
add_subdirectory(applications)

add_git_submodule(schc/full-sdk-delivery)

project(schc_work C)
