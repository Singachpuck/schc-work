add_library(${SCHC_COMP_EXT_API} OBJECT
    "src/fullsdkextapi.c"
#    "src/innerrules.c"
#    "src/outerrules.c"
)

#target_include_directories(${SCHC_COMP_EXT_API} PRIVATE ${FULLSDK_INC_DIR})
target_include_directories(${SCHC_COMP_EXT_API} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${SCHC_COMP_EXT_API} PRIVATE ${FULLSDK_LIB_NAME})