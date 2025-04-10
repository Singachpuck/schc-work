function(add_git_submodule dir PROPAGATE)
    if (NOT DEFINED PROPAGATE)
        option(PROPAGATE "Add cloned project as cmake submodule" ON)
    endif ()

    find_package(Git REQUIRED)

    set(FULL_DIR ${CMAKE_SOURCE_DIR}/${dir})

    file(GLOB DIR_CONTENTS "${FULL_DIR}/*")
    list(LENGTH DIR_CONTENTS RES_LEN)

    if (RES_LEN LESS_EQUAL 1)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${FULL_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif ()

    if (PROPAGATE)
        add_subdirectory(${FULL_DIR})
    endif ()
endfunction()