# Overwrites the default config parameters from SCHC SDK
set(PLATFORM "linux" CACHE STRING "" )
set(TOOLCHAIN "gcc-native" CACHE STRING "" )
set(TARGET "default" CACHE STRING "" )
set(FRAGMENTATION_API "p2p" CACHE STRING "" )
set(EXTENSION_API "default" CACHE STRING "" )

# Custom params
set(APP_NAME "schc_worker_linux" CACHE STRING "" )
set(SCHC_COMP_EXT_API "oscore_basic" CACHE STRING "" )
set(SCHC_FRAG_EXT_API "na_basic" CACHE STRING "" )
set(L2_STACK "ahoi_posix_host" CACHE STRING "" )
option(OSCORE_PROXY_ENABLED "dqwdqwdw" ON)
set(OSCORE_INNER_MAX_SIZE 127 CACHE STRING "" )