cmake_minimum_required(VERSION 3.21)

project(schc_work C)

include(config.cmake)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
#include(AddGitSubmodule)

#set(SCHC_DIR "schc/full-sdk-delivery")
set(DELIVERY_DIR "${CMAKE_BINARY_DIR}/${TOOLCHAIN}/${TARGET}/deliver")
#add_git_submodule(${SCHC_DIR} FALSE)
#message(STATUS "SCHC Full SDK is available at ${SCHC_DIR}")
#
#include(${CMAKE_CURRENT_SOURCE_DIR}/${SCHC_DIR}/full-sdk/toolchains/${TOOLCHAIN}/${TARGET}.cmake)

set(L2_STACK_LIB "l2-ahoi-posix")
set(PLATFORM_LIB "platform-${PLATFORM}")
set(AHOI_LIB "ahoi-module::ahoi-module")
set(IO_LIB "io-lib::io-lib")
set(ASCON_LIB "ascon-c::ascon-c")

include(GNUInstallDirs)
include(profiles/${PROFILE_NAME}/config.cmake)

find_package(ahoi-module)
find_package(ascon-c)

add_subdirectory(schc/schc-sdk-fork)

# Sets APP_DIRECTORY and EXEC_NAMES
add_subdirectory(applications/delivery)

add_subdirectory(configure_files)

add_subdirectory(libs)

include("${PROJECT_SOURCE_DIR}/platforms/${PLATFORM}/flags.cmake")
include("${L2_STACK_BASE}/${L2_STACK}/flags.cmake")
set(FULLSDK_INC_DIR ${CMAKE_BINARY_DIR}/${TOOLCHAIN}/${TARGET}/deliver)
add_subdirectory(platforms/${PLATFORM})
add_subdirectory(${L2_STACK_BASE}/${L2_STACK} l2/${L2_STACK})

add_subdirectory(applications/schc_worker_linux)

#add_subdirectory(${SCHC_DIR})

find_package(cmocka)

add_subdirectory(tests)
