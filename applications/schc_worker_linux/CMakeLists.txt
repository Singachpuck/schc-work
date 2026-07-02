include(config.cmake)

set(EXEC_NAME "schc-worker-linux")

add_subdirectory(config_files)

add_executable(${EXEC_NAME})

#message(STATUS "${FULLSDK_LIB_NAME}")

set(OSCORE_PROXY_LIB "oscore-proxy")

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
        $<TARGET_OBJECTS:${ZCBOR_LIB}>
)
target_link_libraries(${EXEC_NAME} PRIVATE
        ${FULLSDK_LIB_NAME}
        ${IO_LIB}
        ${AHOI_LIB}
        ${NET_LIB}
        ${OSCORE_LIB}
        ${CRYPTO_SUITE}
        ${UTIL_LIB}
)
target_include_directories(${EXEC_NAME} PRIVATE
        ${L2_INC_DIR}
        ${PLATFORM_INC_DIR}
        "${CMAKE_CURRENT_BINARY_DIR}/config_files"
        "${CMAKE_CURRENT_LIST_DIR}/include"
        ${SCHC_COMP_EXT_API_INC_DIR}
        ${SCHC_FRAG_EXT_API_INC_DIR}
)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    target_link_options(${EXEC_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address,undefined>)
endif ()

# Installation

install(TARGETS ${EXEC_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(PROGRAMS
    ${PROJECT_SOURCE_DIR}/tools/run_ipv6_whitelist.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
