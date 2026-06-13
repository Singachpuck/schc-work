# Default config, see profiles/
set(PROFILE_NAME "ahoi_schc_al_oscore_basic" CACHE STRING "Profile name.")

include("profiles/${PROFILE_NAME}/config.cmake")

set(APP_NAME "schc_al_linux" CACHE STRING "")

# Overwrites the default config parameters from SCHC SDK
set(PLATFORM "linux" CACHE STRING "")
set(L2_STACK "ahoi_posix_host" CACHE STRING "")
set(TOOLCHAIN "gcc-native" CACHE STRING "")
set(TARGET "default" CACHE STRING "")
#set(FRAGMENTATION_API "p2p" CACHE STRING "")
set(EXTENSION_API "nocomp" CACHE STRING "")

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}" CACHE PATH "")