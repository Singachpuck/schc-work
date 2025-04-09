set(TARGET_SOURCES "src/measurement_service.c" "device.c")

add_library(${APP_LIB}
        OBJECT
        ${TARGET_SOURCES})

target_include_directories(${APP_LIB}
        PRIVATE
        ./include
        ${PROJECT_SOURCE_DIR}/full-sdk/trace/include
        ${FULLSDK_INC_DIR}
        ${PLATFORM_INC_DIR}
        ${L2_INC_DIR})

target_link_libraries(${APP_LIB} PUBLIC ${COMMONS_LIB} ${SCHC_COMMONS_LIB})
set(EXTRA_LIBS ${COMMONS_LIB} ${SCHC_COMMONS_LIB} PARENT_SCOPE)