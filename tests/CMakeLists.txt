if(NOT TARGET fullsdk)
  message(WARNING "fullsdk target not available; skipping rule-id cmocka test")
  return()
endif()

if(NOT TARGET cmocka::cmocka)
  message(WARNING "cmocka target not available; skipping rule-id cmocka test")
  return()
endif()

enable_testing()

set(TEST_SCHC_PUSH_PULL test_schc_push_pull)
set(TEST_RULE_ID_SIZES 1 5 8 12 16)

set(TRACE_ENABLED OFF CACHE BOOL "Trace" FORCE)
set(RULEPARSER_MEM_TOOL OFF CACHE BOOL "Enables the compilation of a tool to display a summary of the rules parsed by the ruleparser module" FORCE)
set(PERF_MEASURE_ENABLED OFF CACHE BOOL "Enables the performance measurement API which allows measuring the number of CPU instructions used between 2 points in the code" FORCE)

foreach(TEST_RULE_ID_BITS IN LISTS TEST_RULE_ID_SIZES)
  set(TEST_RULE_ID_LIB test_fullsdk_rule_${TEST_RULE_ID_BITS})

  set(FULLSDK_LIB_NAME ${TEST_RULE_ID_LIB} CACHE STRING "Library's name" FORCE)
  set(RULE_ID_BITS ${TEST_RULE_ID_BITS} CACHE STRING "Size of rule IDs. Max 16" FORCE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork
                   ${CMAKE_BINARY_DIR}/schc-sdk-fork-rule-${TEST_RULE_ID_BITS})

  set(TEST_RULE_ID_EXEC test_rule_id_size_${TEST_RULE_ID_BITS})
  add_executable(${TEST_RULE_ID_EXEC} test_rule_id_var_size.c
                                     test_helpers/schc_mocks.c)
  target_compile_definitions(${TEST_RULE_ID_EXEC} PRIVATE RULE_ID_BITS=${TEST_RULE_ID_BITS})
  target_link_libraries(${TEST_RULE_ID_EXEC} PRIVATE ${TEST_RULE_ID_LIB} cmocka::cmocka)
  target_include_directories(
    ${TEST_RULE_ID_EXEC}
    PRIVATE ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/comp/include
            ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/comp/payload
            ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/utils/include
            ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/libs/cmocka/include)

  add_test(NAME ${TEST_RULE_ID_EXEC} COMMAND ${TEST_RULE_ID_EXEC})
endforeach()

set(FULLSDK_LIB_NAME fullsdk CACHE STRING "Library's name" FORCE)
set(TRACE_ENABLED OFF CACHE STRING "Trace" FORCE)
set(RULEPARSER_MEM_TOOL OFF CACHE BOOL "Enables the compilation of a tool to display a summary of the rules parsed by the ruleparser module" FORCE)
set(PERF_MEASURE_ENABLED OFF CACHE BOOL "Enables the performance measurement API which allows measuring the number of CPU instructions used between 2 points in the code" FORCE)
set(RULE_ID_BITS 4 CACHE STRING "Size of rule IDs. Max 16" FORCE)

##############################

add_executable(
  ${TEST_SCHC_PUSH_PULL}
  test_schc_push_pull.c
  test_helpers/packet_builders.c
  test_helpers/schc_mocks.c)
target_link_libraries(${TEST_SCHC_PUSH_PULL} PRIVATE fullsdk cmocka::cmocka)
target_include_directories(
  ${TEST_SCHC_PUSH_PULL}
  PRIVATE ${PROJECT_SOURCE_DIR}/tests
          ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/comp/include
          ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/comp/payload
          ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/utils/include
          ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/templates/include
          ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/core/include
          ${PROJECT_SOURCE_DIR}/schc/schc-sdk-fork/libs/cmocka/include)

add_test(NAME ${TEST_SCHC_PUSH_PULL} COMMAND ${TEST_SCHC_PUSH_PULL})

