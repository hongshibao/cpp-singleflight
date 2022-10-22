#
# Project specific options :
# - SINGLEFLIGHT_BUILD_SHARED_LIB
# - SINGLEFLIGHT_BUILD_TESTS (requires BUILD_TESTING set to ON)
# - SINGLEFLIGHT_INSTALL
# Other options might be available through the cmake scripts including (not exhaustive):
# - ENABLE_WARNINGS_SETTINGS
# - ENABLE_LTO
#

cmake_minimum_required(VERSION 3.14)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# Put the project early since modules might need to detect the compiler.
# More information https://cmake.org/cmake/help/latest/command/project.html
project(
    "SingleFlight"
    VERSION 1.0.0
    LANGUAGES C CXX
)

# ###########################
# #  Modules and scripts   ##
# ###########################

# Standard CMake modules
include(CTest) # Must be called before adding tests but after calling project(). This automatically calls enable_testing() and configures ctest targets when using Make/Ninja
include(CMakeDependentOption) # This is a really useful scripts that creates options that depends on other options. It can even be used with generator expressions !
include(GNUInstallDirs) # This will define the default values for installation directories (all platforms even if named GNU)
include(InstallRequiredSystemLibraries) # Tell CMake that the `install` target needs to install required system libraries (eg: Windows SDK)
include(CMakePackageConfigHelpers) # Helper to create relocatable packages

# Custom modules and scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") # Make our cmake scripts available
include(Warnings)

# ##############
# #  OPTIONS  ##
# ##############

# option(USE_CONAN "Whether to use conan or vcpkg" ON)
# option(ENABLE_COVERAGE "Enable a Code Coverage build." OFF)
# option(ENABLE_CLANG_TIDY "Enable to add clang tidy." OFF)
# option(ENABLE_CPPCHECK "Enable to add cppcheck." OFF)
# option(ENABLE_LTO "Enable to add Link Time Optimization." OFF)
# option(ENABLE_CCACHE "Enable to add Ccache." OFF)
option(SINGLEFLIGHT_BUILD_SHARED_LIB "Build ${PROJECT_NAME} project shared library." ON)
option(SINGLEFLIGHT_INSTALL "Should ${PROJECT_NAME} project be added to the install list? Useful if included using add_subdirectory." ON)
option(SINGLEFLIGHT_BUILD_EXAMPLES "Build ${PROJECT_NAME} project examples." OFF)
cmake_dependent_option(SINGLEFLIGHT_BUILD_TESTS
    "Build ${PROJECT_NAME} project unit tests." ON # By default we want tests if CTest is enabled
    "BUILD_TESTING" OFF # Stay coherent with CTest variables
)

# Global CMake variables are set here
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# When building a shared library, you do not want to export all symbols by default
# gcc (and hence clang) are wrong about this.
#
# For more information, see https://gcc.gnu.org/wiki/Visibility and https://www.youtube.com/embed/m0DwB4OvDXk
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# ##############
# #  Project  ##
# ##############

# =====================#
# singleflight library #
# =====================#
set(LIBRARY_NAME singleflight) # Name for the library built from src/*.cpp

set(SRC_FILES # All .cpp files in src/
    ${CMAKE_CURRENT_LIST_DIR}/src/singleflight.cpp
)
set(INCLUDE_DIR
    ${CMAKE_CURRENT_LIST_DIR}/include)

if(SINGLEFLIGHT_BUILD_SHARED_LIB)
    add_library(${LIBRARY_NAME} SHARED ${SRC_FILES})
else()
    add_library(${LIBRARY_NAME} STATIC ${SRC_FILES})
endif()

# Since we put the public interface headers in the include directory, we need to tell the compiler so that we can #include <file>.
target_include_directories(${LIBRARY_NAME}
    PUBLIC # The folder must be used in the include path for any target using this library
    $<BUILD_INTERFACE:${INCLUDE_DIR}> # Due to the way installation work, we only want this path set when building, not once installed
)

# Add an executable for the file app/main.cpp.
# If you add more executables, copy these lines accordingly.
# add_executable(main app/main.cpp) # Name of exec. and location of file.
# target_link_libraries(main PRIVATE ${LIBRARY_NAME}) # Link the executable to library (if it uses it).

# target_set_warnings(main ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).
# target_enable_lto(main optimized) # enable link-time-optimization if available for non-debug configurations

# Set the properties you require, e.g. what C++ standard to use.
set_target_properties(
    ${LIBRARY_NAME}
    PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# The following properties are useful when you want to have an 'install' target
# set_target_properties(${LIBRARY_NAME}
# PROPERTIES
# PUBLIC_HEADER ${CMAKE_CURRENT_LIST_DIR}/include/singleflight/singleflight.h # Headers listed here will automatically be copied when installing. Note that directories hierarchy is not preserved.
# DEBUG_POSTFIX d # We had a postfix so that we can install debug and release libraries side by side (Windows way)
# )

# For custom settings.
target_compile_definitions(${LIBRARY_NAME} PUBLIC SINGLEFLIGHT_COMPILED_LIB)

# Give a 'namespaced' name to libraries targets, as it can't be mistaken with system libraries
# Use the same namespace as the one from the install command. This is only really needed if you want to support usage of your library through add_subdirectory.
add_library(${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

# CMake scripts extensions
target_set_warnings(${LIBRARY_NAME} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Helper that can set default warning flags for you

# ======#
# Tests #
# ======#

# Set up tests (see tests/CMakeLists.txt).
if(SINGLEFLIGHT_BUILD_TESTS)
    message(STATUS "Adding tests subdirectory")
    add_subdirectory(tests)
endif()

# =========#
# Examples #
# =========#

# Set up examples (see examples/CMakeLists.txt).
if(SINGLEFLIGHT_BUILD_EXAMPLES)
    message(STATUS "Adding examples subdirectory")
    add_subdirectory(examples)
endif()

# ##############
# # Packaging ##
# ##############
if(SINGLEFLIGHT_INSTALL)
    # If we want to use CPack, we need to include it so that it populates variables from our CMakeLists.txt.
    # This will also create a `package` target on supported build systems (make, ninja, VS).
    # There are various CPACK_* variables you can set before `include(CPack)` to configure it (see https://cmake.org/cmake/help/latest/module/CPack.html#variables-common-to-all-cpack-generators).
    set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_LIST_DIR}/README.md)
    include(CPack)

    # Let users choose where to install the cmake package descriptions
    # For that we make use of the CMake Cache
    set(SINGLEFLIGHT_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "Path to install ${PROJECT_NAME} Config*.cmake files to.")
    set(SINGLEFLIGHT_MODULE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake" CACHE STRING "Path to install ${PROJECT_NAME}'s .cmake  module files to.")

    # Use version checking helper provided by CMake so that users can safely use a version number in their find_package calls
    write_basic_package_version_file(
        ${PROJECT_NAME}ConfigVersion.cmake # The name of the version file needed by find_package.
        VERSION ${PROJECT_VERSION} # The version of the project, already set by the `project` command at the top of this file
        COMPATIBILITY SameMajorVersion # We use semantic versioning, backward compatibity is only guaranteed for a same major version
    )

    # We will need our own file if we have our own dependencies or want some special behavior when the user calls find_package
    # otherwise we could simply install the exports as the ${PROJECT_NAME}Config.cmake
    configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in # This is your template file
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake # This is the resulting file
        INSTALL_DESTINATION ${SINGLEFLIGHT_INSTALL_CMAKEDIR} # This is where the file will be installed

        # List of paths that needs to be relocated once installed
        # For example if the variable containing the path is named MY_PATH, all instances of @PACKAGE_MY_PATH@ in the template will be replaced by the relocated version of the path
        # This is mostly only needed when you want to install cmake modules or have an unusual layout that cmake is not aware of.
        PATH_VARS SINGLEFLIGHT_MODULE_INSTALL_DIR # This will be exposed as @PACKAGE_SingleFlight_MODULE_INSTALL_DIR@ in the template file

        # Imported targets do not require the following macros
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # The following will export the targets under the name ${PROJECT_NAME}_Targets, not install them yet
    # It will then need a call to `install(EXPORT)`
    install(
        TARGETS
        ${LIBRARY_NAME} # Install libraries
        EXPORT ${PROJECT_NAME}_Targets

        # If you want to split between runtime and dev for examples, take a look at COMPONENT, NAMELINK_COMPONENT etc
        # More info in Craig Scott's talk "Deep CMake for library authors" https://www.youtube.com/watch?v=m0DwB4OvDXk
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # This time, install all the exported targets under the ${PROJECT_NAME}_Targets name.
    install(
        EXPORT ${PROJECT_NAME}_Targets
        NAMESPACE ${PROJECT_NAME}:: # Always specify a namespace so that users can make sure they link targets with transitive properties and not only the library
        FILE ${PROJECT_NAME}Targets.cmake # This is the file that needs to be included from your *Config.cmake. Otherwise, you could just make this your actual *Config.cmake file.
        DESTINATION ${SINGLEFLIGHT_INSTALL_CMAKEDIR}
    )

    # So far we only installed the exported targets, now install the package config files.
    # If you do not list headers in the PUBLIC_HEADER property, you will need to copy them using `install(FILES)` or `install(DIRECTORY)` too.
    # In that case, you can use CMAKE_INSTALL_INCLUDEDIR as the base destination path.
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
        ${SINGLEFLIGHT_INSTALL_CMAKEDIR}
    )
    install(DIRECTORY
        ${INCLUDE_DIR}/
        DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# ===================================================

# ## CMAKE MODULES
# set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
# include(ConfigSafeGuards)
# include(Warnings)
# include(Docs)
# include(AddGitSubmodule)
# include(FetchContent)

# include(StaticAnalyzers)
# include(LTO)
# include(Ccache)

# if(ENABLE_TESTING)
# include(CTest)
# endif()

# if(ENABLE_COVERAGE)
# include(CodeCoverage)
# append_coverage_compiler_flags()
# endif()
# if(ENABLE_LTO)
# find_lto(CXX)
# endif()

# ## EXTERNAL LIBRARIES
# add_git_submodule(external/json)

# FetchContent_Declare(
# fmt
# GIT_REPOSITORY https://github.com/fmtlib/fmt.git
# GIT_TAG 8.1.1
# GIT_SHALLOW TRUE)
# FetchContent_MakeAvailable(fmt)

# FetchContent_Declare(
# spdlog
# GIT_REPOSITORY https://github.com/gabime/spdlog
# GIT_TAG v1.9.2
# GIT_SHALLOW TRUE)
# FetchContent_MakeAvailable(spdlog)

# if(USE_CONAN)
# message("Using Conan")
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()
# else()
# # Note: VCPKG need more adaptations in cmake files
# # Also, rename the _vcpkg.json file to vcpkg.json
# message("Using VCPKG")
# find_package(Catch2 CONFIG REQUIRED)
# find_package(cxxopts CONFIG REQUIRED)
# endif()

# ## SUB DIRECTORIES
# add_subdirectory(configured)
# add_subdirectory(external)
# add_subdirectory(src)

# add_subdirectory(app)
# add_subdirectory(tests)

# ## INSTALL TARGETS
# install(TARGETS main
# EXPORT ${LIBRARY_NAME}
# ARCHIVE DESTINATION lib
# LIBRARY DESTINATION lib
# RUNTIME DESTINATION bin)

# install(TARGETS ${LIBRARY_NAME}
# ARCHIVE DESTINATION lib
# LIBRARY DESTINATION lib)
