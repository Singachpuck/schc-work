# Custom params
set(APP_NAME "schc_worker_linux" CACHE STRING "" )
set(SCHC_COMP_EXT_API "oscore_basic" CACHE STRING "" )
set(SCHC_FRAG_EXT_API "na_basic" CACHE STRING "" )

include(${CMAKE_CURRENT_LIST_DIR}/../common/udp6.cmake)

option(OSCORE_PROXY_ENABLED "" ON)
set(OSCORE_INNER_MAX_SIZE 127 CACHE STRING "" )