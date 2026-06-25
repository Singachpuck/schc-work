add_library(${ASCON_INTEGRATION_LIB} OBJECT "src/ascon_integration.c")
target_include_directories(${ASCON_INTEGRATION_LIB} PRIVATE ${OSCORE_INC})
target_link_libraries(${ASCON_INTEGRATION_LIB} PRIVATE ${ASCON_LIB})