# Default config, see profiles/
#set(PROFILE_NAME "udp6_schc_al_oscore" CACHE STRING "Profile name.")
set(PROFILE_NAME "ahoi_schc_al_oscore" CACHE STRING "Profile name.")

include("profiles/${PROFILE_NAME}/config.cmake")

set(APP_NAME "schc_al_linux" CACHE STRING "")

# Overwrites the default config parameters from SCHC SDK
set(PLATFORM "linux" CACHE STRING "")
set(L2_STACK_BASE "${CMAKE_CURRENT_SOURCE_DIR}/l2" CACHE STRING "")
set(L2_STACK "ahoi_posix_host" CACHE STRING "")
set(TOOLCHAIN "gcc-native" CACHE STRING "")
set(TARGET "default" CACHE STRING "")
set(COMP_API_BASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/schc_comp_apis" CACHE STRING "")
set(FRAGMENTATION_API "nocomp" CACHE STRING "")
set(FRAG_API_BASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/schc_frag_apis" CACHE STRING "")
set(EXTENSION_API "default" CACHE STRING "")

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}" CACHE PATH "")

#set(APP_NAME "schc_worker_linux" CACHE STRING "" )
#set(SCHC_COMP_EXT_API "oscore_basic" CACHE STRING "" )
#set(SCHC_FRAG_EXT_API "na_basic" CACHE STRING "" )
#set(L2_STACK "ahoi_posix_host" CACHE STRING "" )
#option(OSCORE_PROXY_ENABLED "dqwdqwdw" ON)
#set(OSCORE_INNER_MAX_SIZE 127 CACHE STRING "" )