add_library(${APP_LIB}
        OBJECT
        ${CMAKE_CURRENT_LIST_DIR}/schc_core.c)

target_include_directories(${APP_LIB}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/full-sdk/trace/include
        ${FULLSDK_INC_DIR}
        ${PLATFORM_INC_DIR}
        ${L2_INC_DIR})

target_link_libraries(${APP_LIB} PUBLIC ${COMMONS_LIB} ${SCHC_COMMONS_LIB})
set(EXTRA_LIBS ${COMMONS_LIB} ${SCHC_COMMONS_LIB} PARENT_SCOPE)