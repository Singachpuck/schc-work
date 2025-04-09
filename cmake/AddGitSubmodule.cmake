function(add_git_submodule dir PROPAGATE)
    if (NOT DEFINED PROPAGATE)
        option(PROPAGATE "Add cloned project as cmake submodule" ON)
    endif ()

    find_package(Git REQUIRED)

    if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${CMAKE_SOURCE_DIR}/${dir}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif ()

    message(STATUS "SCHC Full SDK is available at ${dir}")

    if (PROPAGATE)
        add_subdirectory(${dir})
    endif ()
endfunction()