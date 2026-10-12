# Custom params
set(APP_NAME "schc_worker_linux" CACHE STRING "" )
set(SCHC_COMP_EXT_API "oscore_basic" CACHE STRING "" )
set(SCHC_FRAG_EXT_API "na_basic" CACHE STRING "" )

include(${CMAKE_CURRENT_LIST_DIR}/../common/ahoi.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../common/core.cmake) # SCHC_CORE_MODE
include(${CMAKE_CURRENT_LIST_DIR}/../common/oscore.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../common/ascon.cmake)
