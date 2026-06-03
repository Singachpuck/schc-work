add_library(
        ${L2_STACK_LIB}
        OBJECT
#        ${CMAKE_CURRENT_LIST_DIR}/src/fullsdkl2.c
        ${CMAKE_CURRENT_LIST_DIR}/src/fullsdkl2a.c
)

target_include_directories(${L2_STACK_LIB}
        PRIVATE
        ${L2_INC_DIR}
        ${FULLSDK_INC_DIR}
        ${PLATFORM_INC_DIR})

target_link_libraries(${L2_STACK_LIB} PRIVATE ${AHOI_LIB} ${IO_LIB})