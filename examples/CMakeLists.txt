cmake_minimum_required(VERSION 3.14)

# List all files containing examples.
set(EXAMPLE_FILES # All .cpp files in examples/
    ${CMAKE_CURRENT_LIST_DIR}/example.cpp
)

set(EXAMPLE_MAIN example) # Name for example executable.

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

add_executable(${EXAMPLE_MAIN} ${EXAMPLE_FILES})
target_link_libraries(${EXAMPLE_MAIN} PRIVATE ${LIBRARY_NAME} Threads::Threads spdlog::spdlog)
set_target_properties(
    ${EXAMPLE_MAIN}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_set_warnings(${EXAMPLE_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings.
