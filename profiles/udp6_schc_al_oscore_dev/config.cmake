# Custom params
set(APP_NAME "schc_worker_linux" CACHE STRING "" )
set(SCHC_COMP_EXT_API "oscore_basic" CACHE STRING "" )
set(SCHC_FRAG_EXT_API "na_basic" CACHE STRING "" )

set(L2_UDP6_HOST_IP "::" CACHE STRING "")
set(L2_UDP6_REMOTE_IP "::1" CACHE STRING "")
set(L2_UDP6_HOST_PORT "4050" CACHE STRING "")
set(L2_UDP6_REMOTE_PORT "4051" CACHE STRING "")

include(${CMAKE_CURRENT_LIST_DIR}/../common/udp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../common/dev.cmake) # SCHC DEV MODE
include(${CMAKE_CURRENT_LIST_DIR}/../common/oscore.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../common/ascon.cmake)
