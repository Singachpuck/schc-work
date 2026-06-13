include(config.cmake)

set(EXEC_NAME "schc-worker-linux")

add_subdirectory(config_files)

add_executable(${EXEC_NAME})

#message(STATUS "${FULLSDK_LIB_NAME}")

include(ExternalProject)

set(OSCORE_LIB "uoscore")
set(OSCORE_PROXY_LIB "oscore-proxy")
set(UOSCORE_SRC_DIR "${PROJECT_SOURCE_DIR}/libs/external/uoscore")
set(UOSCORE_BUILD_DIR "${UOSCORE_SRC_DIR}/build")

# uOSCORE dependencies
set(MBEDCRYPTO_LIB "mbedcrypto")
set(ZCBOR_INC "${UOSCORE_SRC_DIR}/externals/zcbor/include/")
set(ZCBOR_SRC
        "${UOSCORE_SRC_DIR}/externals/zcbor/src/zcbor_common.c"
        "${UOSCORE_SRC_DIR}/externals/zcbor/src/zcbor_encode.c")
add_subdirectory("${UOSCORE_SRC_DIR}/externals/mbedtls" "${PROJECT_BINARY_DIR}/mbedtls")

ExternalProject_Add(uoscore_project
    SOURCE_DIR "${UOSCORE_SRC_DIR}"
    BINARY_DIR "${UOSCORE_SRC_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR}
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${UOSCORE_BUILD_DIR}/libuoscore-uedhoc.a"
)

add_library(${OSCORE_LIB} STATIC IMPORTED GLOBAL)
set_target_properties(${OSCORE_LIB} PROPERTIES
    IMPORTED_LOCATION "${UOSCORE_BUILD_DIR}/libuoscore-uedhoc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${UOSCORE_SRC_DIR}/inc"
)
add_dependencies(${OSCORE_LIB} uoscore_project)

add_library(${OSCORE_PROXY_LIB} OBJECT
        "${CMAKE_CURRENT_SOURCE_DIR}/src/uoscore_integration.c")
target_include_directories(${OSCORE_PROXY_LIB} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/config_files"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${PLATFORM_INC_DIR}
        ${NET_INC_DIR}
        ${SCHC_COMP_EXT_API_INC_DIR}
        ${FULLSDK_INC_DIR}
)
target_link_libraries(${OSCORE_PROXY_LIB} PRIVATE ${OSCORE_LIB})
target_sources(${EXEC_NAME} PRIVATE
        $<TARGET_OBJECTS:${OSCORE_PROXY_LIB}>)

# Exec

target_sources(${EXEC_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/schc_al.c"
        $<TARGET_OBJECTS:${L2_STACK_LIB}>
        $<TARGET_OBJECTS:${PLATFORM_LIB}>
        $<TARGET_OBJECTS:${SCHC_COMP_EXT_API}>
        $<TARGET_OBJECTS:${SCHC_FRAG_EXT_API}>
        ${ZCBOR_SRC}
)
target_link_libraries(${EXEC_NAME} PRIVATE
        ${FULLSDK_LIB_NAME}
        ${IO_LIB}
        ${AHOI_LIB}
        ${NET_LIB}
        ${OSCORE_LIB}
        ${MBEDCRYPTO_LIB}
)
target_include_directories(${EXEC_NAME} PRIVATE
        ${L2_INC_DIR}
        ${PLATFORM_INC_DIR}
        "${CMAKE_CURRENT_BINARY_DIR}/config_files"
        "${CMAKE_CURRENT_LIST_DIR}/include"
        ${SCHC_COMP_EXT_API_INC_DIR}
        ${SCHC_FRAG_EXT_API_INC_DIR}
        ${ZCBOR_INC}
)

# Installation

install(TARGETS ${EXEC_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(PROGRAMS
    ${PROJECT_SOURCE_DIR}/tools/run_ipv6_whitelist.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
