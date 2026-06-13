add_library(${SCHC_FRAG_EXT_API} OBJECT
        "src/fullsdkfragapi.c"
)

target_link_libraries(${SCHC_FRAG_EXT_API} PRIVATE ${FULLSDK_LIB_NAME})