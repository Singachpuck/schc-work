# Overwrites the default config parameters from SCHC SDK

set(APP_NAME "lpwan_model")

set(PLATFORM "linux")
set(L2_STACK "udp")
set(TOOLCHAIN "gcc-native")
set(TARGET "default")
set(FRAGMENTATION_API "p2p")
set(EXTENSION_API "template")
set(TEMPLATE_ID "ipv6udp")
set(TEMPLATE_FILE "${CMAKE_SOURCE_DIR}/templates/custom_template.c")
set(TRACE_ENABLED ON)