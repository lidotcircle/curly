
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    DOWNLOAD_ONLY ON
    GIT_TAG 05CC6081FCBD0071053DE78238E136B3
)

if(googletest_ADDED)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

