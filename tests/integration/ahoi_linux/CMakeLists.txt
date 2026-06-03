
find_package(ahoi-module REQUIRED)

set(TEST_SCHC_AHOI_L2 "test-l2-ahoi")

set(TRACE_ENABLED OFF CACHE BOOL "Trace" FORCE)
set(RULEPARSER_MEM_TOOL OFF CACHE BOOL "Enables the compilation of a tool to display a summary of the rules parsed by the ruleparser module" FORCE)
set(PERF_MEASURE_ENABLED OFF CACHE BOOL "Enables the performance measurement API which allows measuring the number of CPU instructions used between 2 points in the code" FORCE)
set(FULLSDK_LIB_NAME test-fullsdk-ahoi-linux CACHE STRING "Library's name" FORCE)
set(RULE_ID_BITS 4 CACHE STRING "Size of rule IDs. Max 16" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork ${PROJECT_BINARY_DIR}/schc-sdk-ahoi-linux)

set(L2_STACK "ahoi_posix_host")
set(L2_STACK_LIB "l2-ahoi-posix")
set(PLATFORM "linux")
set(PLATFORM_LIB "platform-${PLATFORM}")
set(AHOI_LIB "ahoi-module::ahoi-module")
set(IO_LIB "io-lib::io-lib")

include(${PROJECT_SOURCE_DIR}/platforms/${PLATFORM}/flags.cmake)
include(${PROJECT_SOURCE_DIR}/l2/${L2_STACK}/flags.cmake)
set(FULLSDK_INC_DIR ${CMAKE_BINARY_DIR}/${TOOLCHAIN}/${TARGET}/deliver)

add_subdirectory(${PROJECT_SOURCE_DIR}/l2/${L2_STACK} ${CMAKE_BINARY_DIR}/l2/${L2_STACK})
add_subdirectory(${PROJECT_SOURCE_DIR}/platforms/${PLATFORM} ${CMAKE_BINARY_DIR}/platform/${PLATFORM})


add_executable(
    ${TEST_SCHC_AHOI_L2}
    test_l2_ahoi.c
    ${PROJECT_SOURCE_DIR}/tests/test_helpers/packet_builders.c
    $<TARGET_OBJECTS:${L2_STACK_LIB}>
    $<TARGET_OBJECTS:${PLATFORM_LIB}>
)
target_link_libraries(${TEST_SCHC_AHOI_L2} PRIVATE ${FULLSDK_LIB_NAME} ${IO_LIB} ${AHOI_LIB}  cmocka::cmocka)
target_include_directories(
    ${TEST_SCHC_AHOI_L2} PRIVATE
    ${L2_INC_DIR}
    ${PLATFORM_INC_DIR}
)

add_test(NAME ${TEST_SCHC_AHOI_L2} COMMAND ${TEST_SCHC_AHOI_L2})

set(FULLSDK_LIB_NAME fullsdk CACHE STRING "Library's name" FORCE)
set(TRACE_ENABLED OFF CACHE STRING "Trace" FORCE)
set(RULEPARSER_MEM_TOOL OFF CACHE BOOL "Enables the compilation of a tool to display a summary of the rules parsed by the ruleparser module" FORCE)
set(PERF_MEASURE_ENABLED OFF CACHE BOOL "Enables the performance measurement API which allows measuring the number of CPU instructions used between 2 points in the code" FORCE)
set(RULE_ID_BITS 4 CACHE STRING "Size of rule IDs. Max 16" FORCE)

