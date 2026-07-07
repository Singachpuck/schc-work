set(UTIL_LIB "util-lib")
set(UTIL_LIB ${UTIL_LIB} PARENT_SCOPE)
set(UTIL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(UTIL_INC_DIR ${UTIL_INC_DIR} PARENT_SCOPE)

add_library(${UTIL_LIB})

target_sources(${UTIL_LIB} PRIVATE
  "src/logging.c"
)

target_include_directories(${UTIL_LIB} PUBLIC
  ${UTIL_INC_DIR}
)