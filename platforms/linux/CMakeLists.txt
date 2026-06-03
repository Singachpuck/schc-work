add_library(${PLATFORM_LIB}
  OBJECT
  ${CMAKE_CURRENT_LIST_DIR}/src/platform.c
  ${CMAKE_CURRENT_LIST_DIR}/src/base64.c)

target_link_libraries(${PLATFORM_LIB} rt pthread)

target_include_directories(${PLATFORM_LIB}
  PRIVATE
  ${PLATFORM_INC_DIR})