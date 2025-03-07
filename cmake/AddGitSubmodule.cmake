function(add_git_submodule dir)
    find_package(Git REQUIRED)

    if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${dir}/CMakeLists.txt)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${CMAKE_SOURCE_DIR}/${dir}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    endif ()

    add_subdirectory(${dir})
endfunction()