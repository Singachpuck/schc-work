add_library(${NET_LIB})

target_sources(${NET_LIB} PRIVATE
    "src/tun.c"
    "src/udp.c"
)

target_include_directories(${NET_LIB} PUBLIC
    ${NET_INC_DIR}
)