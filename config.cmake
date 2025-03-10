# Overwrites the default config parameters from SCHC SDK

set(APP_NAME "lpwan_model")

set(TOOLCHAIN "gcc-native")
set(TARGET "default")
set(FRAGMENTATION_API "p2p")
set(EXTENSION_API "template")
set(TEMPLATE_ID "custom")
set(TEMPLATE_FILE "${CMAKE_SOURCE_DIR}/templates/custom_template.c")
set(TRACE_ENABLED ON)