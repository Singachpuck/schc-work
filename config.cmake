# Overwrites the default config parameters from SCHC SDK

set(APP_NAME "lpwan_model")

set(PLATFORM "linux")
set(L2_STACK "udp")
set(TOOLCHAIN "gcc-native")
set(TARGET "default")
set(FRAGMENTATION_API "p2p" CACHE STRING "")
set(EXTENSION_API "nocomp" CACHE STRING "")
set(TEMPLATE_ID "ipv6udp")
set(TEMPLATE_FILE "${CMAKE_SOURCE_DIR}/templates/custom_template.c")