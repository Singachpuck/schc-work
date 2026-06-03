
set(MULTI_EXEC_APPS)

list(APPEND MULTI_EXEC_APPS "lpwan_model")

if (${APP_NAME} IN_LIST MULTI_EXEC_APPS)
    set(APP_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/${APP_NAME}" PARENT_SCOPE)
    add_subdirectory(${APP_NAME})

    foreach (app_VAR IN LISTS EXPORT_VARIABLES)
        set(${app_VAR} ${${app_VAR}} PARENT_SCOPE)
    endforeach ()
else()
    set(APP_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)
endif ()