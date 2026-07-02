include(ExternalProject)

set(COMMONS_LIB "commons-lib")
set(COMMONS_LIB ${COMMONS_LIB} PARENT_SCOPE)

set(SCHC_COMMONS_LIB "schc-commons-lib")
set(SCHC_COMMONS_LIB ${SCHC_COMMONS_LIB} PARENT_SCOPE)

set(COAP_LIB "coap-3")
set(COAP_LIB ${COAP_LIB} PARENT_SCOPE)

set(SCHC_LIBS ${SCHC_COMMONS_LIB} PARENT_SCOPE)

set(NET_LIB "net-lib")
set(NET_LIB ${NET_LIB} PARENT_SCOPE)
set(NET_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/net/include")
set(NET_INC_DIR ${NET_INC_DIR} PARENT_SCOPE)

set(UTIL_LIB "util-lib")
set(UTIL_LIB ${UTIL_LIB} PARENT_SCOPE)
set(UTIL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/util/include")
set(UTIL_INC_DIR ${UTIL_INC_DIR} PARENT_SCOPE)

if (WITH_COMMONS)
    add_subdirectory(commons)
endif ()

if (WITH_SCHC_COMMONS)
    add_subdirectory(schc_commons)
endif ()

add_subdirectory(net)
set(NET_LIB ${NET_LIB} PARENT_SCOPE)
set(NET_INC_DIR ${NET_INC_DIR} PARENT_SCOPE)

add_subdirectory(util)
set(UTIL_LIB ${UTIL_LIB} PARENT_SCOPE)
set(UTIL_INC_DIR ${UTIL_INC_DIR} PARENT_SCOPE)

if (WITH_COAP)
    set(ENABLE_DTLS OFF)
    set(ENABLE_DOCS OFF)
    set(ENABLE_TESTS OFF)
    set(ENABLE_EXAMPLES OFF)
    add_git_submodule(libs/external/libcoap TRUE)
endif ()

if (DEFINED SCHC_COMP_EXT_API)
    set(SCHC_COMP_EXT_API_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schc_comp_apis/${SCHC_COMP_EXT_API}/include")
    set(SCHC_COMP_EXT_API_INC_DIR ${SCHC_COMP_EXT_API_INC_DIR} PARENT_SCOPE)
    add_subdirectory(schc_comp_apis/${SCHC_COMP_EXT_API})
endif ()

if (DEFINED SCHC_FRAG_EXT_API)
    set(SCHC_FRAG_EXT_API_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schc_frag_apis/${SCHC_FRAG_EXT_API}/include")
    set(SCHC_FRAG_EXT_API_INC_DIR ${SCHC_FRAG_EXT_API_INC_DIR} PARENT_SCOPE)
    add_subdirectory(schc_frag_apis/${SCHC_FRAG_EXT_API})
endif ()

if (OSCORE_PROXY_ENABLED)
    include(ExternalProject)

    set(OSCORE_LIB "uoscore")
    set(UOSCORE_SRC_DIR "${PROJECT_SOURCE_DIR}/libs/external/uoscore")
    set(OSCORE_INC "${UOSCORE_SRC_DIR}/inc")
    set(UOSCORE_BUILD_DIR "${UOSCORE_SRC_DIR}/build")

    # uOSCORE dependencies
#    set(MBEDCRYPTO_LIB "mbedcrypto")
#    add_subdirectory("${UOSCORE_SRC_DIR}/externals/mbedtls" "${PROJECT_BINARY_DIR}/mbedtls")

    set(ZCBOR_LIB "zcbor")
    set(ZCBOR_INC "${UOSCORE_SRC_DIR}/externals/zcbor/include/")
    set(ZCBOR_SRC
            "${UOSCORE_SRC_DIR}/externals/zcbor/src/zcbor_common.c"
            "${UOSCORE_SRC_DIR}/externals/zcbor/src/zcbor_encode.c")
    add_library(${ZCBOR_LIB} OBJECT ${ZCBOR_SRC})
    target_include_directories(${ZCBOR_LIB} PRIVATE ${ZCBOR_INC})

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
            INTERFACE_INCLUDE_DIRECTORIES "${OSCORE_INC}"
    )
    add_dependencies(${OSCORE_LIB} uoscore_project)

    set(OSCORE_INC ${OSCORE_INC} PARENT_SCOPE)
    set(OSCORE_LIB ${OSCORE_LIB} PARENT_SCOPE)
    set(ZCBOR_LIB ${ZCBOR_LIB} PARENT_SCOPE)
endif ()

if (CRYPTO_LIB STREQUAL "ascon-c")
    set(ASCON_INTEGRATION_LIB "ascon-integration")
#    set(ASCON_INTEGRATION_LIB ${ASCON_INTEGRATION_LIB})
    add_subdirectory(ascon_integration)
    set(CRYPTO_SUITE ${ASCON_LIB} $<TARGET_OBJECTS:${ASCON_INTEGRATION_LIB}> PARENT_SCOPE)
endif ()
