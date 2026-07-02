add_library(${UTIL_LIB} INTERFACE)

target_include_directories(${UTIL_LIB} PUBLIC INTERFACE
  ${UTIL_INC_DIR}
)