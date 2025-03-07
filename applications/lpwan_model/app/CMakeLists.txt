add_library(${APP_LIB}
        OBJECT
        ${CMAKE_CURRENT_LIST_DIR}/app.c)

target_include_directories(${APP_LIB}
        PRIVATE
        "../include"
        ${PROJECT_SOURCE_DIR}/full-sdk/trace/include
        ${FULLSDK_INC_DIR}
        ${PLATFORM_INC_DIR}
        ${L2_INC_DIR})