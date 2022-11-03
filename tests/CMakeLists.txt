cmake_minimum_required(VERSION 3.14)

# List all files containing tests.
set(TEST_FILES # All .cpp files in tests/
    ${CMAKE_CURRENT_LIST_DIR}/test.cpp
)

set(TEST_MAIN unit_tests) # Name for test executable.
set(TEST_RUNNER_PARAMS "") # Any arguemnts to feed the test runner.

find_package(Threads REQUIRED)

# Fetch external dependencies
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.10.0
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.1.1
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Catch2)

add_executable(${TEST_MAIN} ${TEST_FILES})
target_link_libraries(${TEST_MAIN} PRIVATE ${LIBRARY_NAME} Threads::Threads Catch2::Catch2WithMain spdlog::spdlog)
set_target_properties(
    ${TEST_MAIN}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_set_warnings(${TEST_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings.

add_test(

    # Use some per-module/project prefix so that it is easier to run only tests for this module
    NAME ${LIBRARY_NAME}.${TEST_MAIN} COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS}
)

# Adds a 'coverage' target.
# include(CodeCoverage)

# # target_link_libraries(${TEST_MAIN} PRIVATE
# # ${CONAN_CATCH2})
# # if(${ENABLE_WARNINGS})
# # target_set_warnings(TARGET ${TEST_MAIN} ENABLE ON AS_ERROR OFF)
# # endif()

# # if(ENABLE_COVERAGE)
# # set(COVERAGE_MAIN "coverage")
# # set(COVERAGE_EXCLUDES
# # "${PROJECT_SOURCE_DIR}/app/*"
# # "${PROJECT_SOURCE_DIR}/cmake/*"
# # "${PROJECT_SOURCE_DIR}/docs/*"
# # "${PROJECT_SOURCE_DIR}/external/*"
# # "${PROJECT_SOURCE_DIR}/tests/*"
# # "${CONAN_CATCH2_ROOT}/*"
# # "${CONAN_CXXOPTS_ROOT}/*"
# # "/usr/include/*")
# # setup_target_for_coverage_lcov(
# # NAME ${COVERAGE_MAIN}
# # EXECUTABLE ${TEST_MAIN}
# # DEPENDENCIES ${TEST_MAIN}
# # )
# # endif()
# endif()
