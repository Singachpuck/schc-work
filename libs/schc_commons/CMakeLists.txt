
add_library(${SCHC_COMMONS_LIB} "src/schc_helper.c")

target_include_directories(${SCHC_COMMONS_LIB} PUBLIC "include" ${DELIVERY_DIR})

target_link_libraries(${SCHC_COMMONS_LIB} ${FULLSDK_LIB_NAME} ${COMMONS_LIB})