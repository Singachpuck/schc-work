
add_library(${COMMONS_LIB} STATIC "src/net_helper.c")

target_include_directories(${COMMONS_LIB} PUBLIC "include")