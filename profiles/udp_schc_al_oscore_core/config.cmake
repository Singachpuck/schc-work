# Custom params
set(APP_NAME "schc_worker_linux" CACHE STRING "" )
set(SCHC_COMP_EXT_API "oscore_basic" CACHE STRING "" )
set(SCHC_FRAG_EXT_API "na_basic" CACHE STRING "" )

set(L2_UDP_HOST_IP "0.0.0.0" CACHE STRING "")
set(L2_UDP_REMOTE_IP "127.0.0.1" CACHE STRING "")
set(L2_UDP_HOST_PORT "4051" CACHE STRING "")
set(L2_UDP_REMOTE_PORT "4050" CACHE STRING "")

include(${CMAKE_CURRENT_LIST_DIR}/../common/udp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../common/core.cmake) # SCHC_CORE_MODE
include(${CMAKE_CURRENT_LIST_DIR}/../common/oscore.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../common/ascon.cmake)
