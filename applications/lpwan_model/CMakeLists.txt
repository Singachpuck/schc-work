set(EXEC_NAMES "app" "device" "schc_core" PARENT_SCOPE)

set(WITH_COMMONS ON PARENT_SCOPE)
set(WITH_SCHC_COMMONS ON PARENT_SCOPE)
set(WITH_COAP ON PARENT_SCOPE)

# List all the variables that need to be exported to the full-sdk-delivery CMakeLists.txt
set(EXPORT_VARIABLES "EXEC_NAMES" "WITH_COMMONS" "WITH_SCHC_COMMONS" "WITH_COAP" PARENT_SCOPE)