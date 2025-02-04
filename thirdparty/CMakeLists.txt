include(FetchContent)
FetchContent_Declare(
        schc
        GIT_REPOSITORY https://gitlab.com/lab-schc/sdk/full-sdk.git
        GIT_TAG main
)
FetchContent_MakeAvailable(schc)