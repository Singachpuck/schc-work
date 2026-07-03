add_library(${UTIL_LIB})

target_sources(${UTIL_LIB} PRIVATE
  "src/logging.c"
)

target_include_directories(${UTIL_LIB} PUBLIC
  ${UTIL_INC_DIR}
)