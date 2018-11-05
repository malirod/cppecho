cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake-common-utils")
include(PreferClang)
include(PreferCcache)
include(DefaultDebug)
include(PreferGoldLinker)

project(flatasync VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Manage dependencies

include(PrepareConan)

# This will set output dir to bin
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)

# Add possibility to sanitize code
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sanitizers-cmake/cmake")
find_package(Sanitizers REQUIRED)

# Set global warnings configuration for all sub-projects
add_compile_options(-Wall -Wextra -pedantic -Werror)

if (BUILD_TESTING)
    # Add possibility to get code coverage
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake-codecov/cmake")
    find_package(codecov REQUIRED)
endif()

# add main sub-projects
add_subdirectory(cppecho)
add_subdirectory(flatasync)

if (BUILD_TESTING)
    # add main test runner
    add_subdirectory(testrunner)
    add_test(NAME all COMMAND testrunner)
endif()

include(ClangTidy)
include(PrepareDoxygen)
include(ClangStaticAnalyzer)

# Copy default config to the output dir
# Manual run of the executable is expected from build dir directly
# ./bin/testrunner
configure_file(config/logger.cfg logger.cfg COPYONLY)