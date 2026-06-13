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

if (WITH_COMMONS)
    add_subdirectory(commons)
endif ()

if (WITH_SCHC_COMMONS)
    add_subdirectory(schc_commons)
endif ()

add_subdirectory(net)
set(NET_LIB ${NET_LIB} PARENT_SCOPE)
set(NET_INC_DIR ${NET_INC_DIR} PARENT_SCOPE)

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