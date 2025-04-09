
set(COMMONS_LIB "commons-lib")
set(COMMONS_LIB ${COMMONS_LIB} PARENT_SCOPE)

set(SCHC_COMMONS_LIB "schc-commons-lib")
set(SCHC_COMMONS_LIB ${SCHC_COMMONS_LIB} PARENT_SCOPE)

set(SCHC_LIBS ${SCHC_COMMONS_LIB} PARENT_SCOPE)

if (WITH_COMMONS)
    add_subdirectory(commons)
endif ()

if (WITH_SCHC_COMMONS)
    add_subdirectory(schc_commons)
endif ()