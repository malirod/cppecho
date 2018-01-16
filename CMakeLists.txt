cmake_minimum_required(VERSION 3.5)

include(cmake/cmake-common-utils/PreferClang.cmake)
include(cmake/cmake-common-utils/PreferCcache.cmake)
include(cmake/cmake-common-utils/DefaultDebug.cmake)
include(cmake/cmake-common-utils/PreferGoldLinker.cmake)

project(cpputils VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

# Manage dependencies

# Setup Conan
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(STATUS "Setup dependencies with conan (clang)")
        execute_process(COMMAND conan install ${CMAKE_CURRENT_LIST_DIR} --profile ${CMAKE_CURRENT_LIST_DIR}/tools/conan/profile-clang --build missing)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(STATUS "Setup dependencies with conan (gcc)")
        execute_process(COMMAND conan install ${CMAKE_CURRENT_LIST_DIR} --profile ${CMAKE_CURRENT_LIST_DIR}/tools/conan/profile-gcc --build missing)
    else()
        set(CONAN_INSTALL_CMD "conan install .. <list-of-settings>")
        message(FATAL_ERROR "Dependencies are not configured. Install dependencies first, e.g. running in build dir: ${CONAN_INSTALL_CMD}")
    endif()
endif()

# This will set output dir to bin
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Add possibility to sinitize code
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/sanitizers-cmake/cmake/")
find_package(Sanitizers REQUIRED)

# Add possibility to get code coverage
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/CMake-codecov/cmake")
find_package(codecov REQUIRED)

include(CTest)

set(SRC_LIST
    "src/core/alias.h"
    "src/core/async.cc"
    "src/core/async.h"
    "src/core/async_op_state.cc"
    "src/core/async_op_state.h"
    "src/core/async_proxy.cc "
    "src/core/async_proxy.h"
    "src/core/async_runner.cc"
    "src/core/async_runner.h"
    "src/core/coro_helper.cc"
    "src/core/coro_helper.h"
    "src/core/default_scheduler_accessor.cc"
    "src/core/default_scheduler_accessor.h"
    "src/core/engine.cc"
    "src/core/engine.h"
    "src/core/engine_config.cc"
    "src/core/engine_config.h"
    "src/core/engine_launcher.cc"
    "src/core/engine_launcher.h"
    "src/core/general_error.cc"
    "src/core/general_error.h"
    "src/core/iengine.h"
    "src/core/iengine_config.h"
    "src/core/iioservice.h"
    "src/core/ischeduler.h"
    "src/core/sequential_scheduler.cc"
    "src/core/sequential_scheduler.h"
    "src/core/startup_config.cc"
    "src/core/startup_config.h"
    "src/core/thread_pool.cc"
    "src/core/thread_pool.h"
    "src/core/version.cc"
    "src/core/version.h"
    "src/net/acceptor.cc"
    "src/net/acceptor.h"
    "src/net/alias.h"
    "src/net/resolver.cc"
    "src/net/resolver.h"
    "src/net/tcp_server.cc"
    "src/net/tcp_server.h"
    "src/net/tcp_socket.cc"
    "src/net/tcp_socket.h"
    "src/net/util.cc"
    "src/net/util.h"
    "src/util/enum_util.h"
    "src/util/logger.cc"
    "src/util/logger.h"
    "src/util/scope_guard.h"
    "src/util/singleton.h"
    "src/util/smartptr_util.h"
    "src/util/static_string.h"
    "src/util/string_fmt.h"
    "src/util/thread_util.cc"
    "src/util/thread_util.h"
    "src/util/type_traits.h")

add_executable(cppecho
    "${SRC_LIST}"
    "src/core/lifecycle.cc")

add_executable(testrunner
    "${SRC_LIST}"
    "test/core/async_proxy_test.cc"
    "test/core/async_test.cc"
    "test/core/coro_helper_test.cc"
    "test/core/coroutine_test.cc"
    "test/core/engine_test.cc"
    "test/core/general_error_test.cc"
    "test/core/helper.cc"
    "test/core/helper.h"
    "test/core/testrunner.cc"
    "test/net/tcp_server_test.cc"
    "test/net/tcp_socket_test.cc"
    "test/util/enum_util_test.cc"
    "test/util/logger_test.cc"
    "test/util/rvo_test.cc"
    "test/util/scope_guard_test.cc"
    "test/util/singleton_test.cc"
    "test/util/static_string_test.cc"
    "test/util/string_fmt_test.cc")

add_sanitizers(testrunner)

add_coverage(testrunner)

# Setup clang-tidy target
find_program(CLANG_TIDY_EXE NAMES "clang-tidy-5.0" PATHS /usr/bin)
find_program(RUN_CLANG_TIDY_EXE NAMES "run-clang-tidy.py" PATHS /usr/lib/llvm-5.0/share/clang)

if (NOT CLANG_TIDY_EXE)
    message(WARNING "clang-tidy not found!")
endif()

if (NOT RUN_CLANG_TIDY_EXE)
    message(WARNING "run-clang-tidy.py not found!")
endif()

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy and run-clang-tidy.py")
    set(CLANG_TIDY_HEADER_FILTER "'\/src\/|\/test\/'")

    add_custom_target(clang-tidy
        COMMAND ${RUN_CLANG_TIDY_EXE} -quiet -header-filter=${CLANG_TIDY_HEADER_FILTER}
        VERBATIM)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Setup include-what-you-use
find_program(IWYU_EXE NAMES "include-what-you-use" PATHS /usr/local/bin)
find_program(IWYU_SCRIPT NAMES "iwyu_tool.py" PATHS /usr/local/bin)
if(NOT IWYU_EXE OR NOT IWYU_SCRIPT)
    message(WARNING "Could not find the program(s) include-what-you-use or iwyu_tool.py")
else()
    message(STATUS "Found include-what-you-use: ${IWYU_EXE}")
    message(STATUS "Found iwyu_tool.py: ${IWYU_SCRIPT}")
    add_custom_target(iwyu
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/tools/iwyu/run_iwyu.sh
        VERBATIM)
endif()

# Setup clang static analyzer
add_custom_target(clang-static-analyzer
    COMMAND scan-build --use-analyzer=/usr/bin/clang++-5.0 make
    VERBATIM)

target_include_directories(testrunner
    PRIVATE
    src
    test)

target_include_directories(cppecho
    PRIVATE
    src)

target_compile_options(testrunner PRIVATE -Wextra -Werror)
target_compile_options(cppecho PRIVATE -Wextra -Werror)

target_link_libraries(testrunner PRIVATE CONAN_PKG::gtest CONAN_PKG::log4cplus CONAN_PKG::Boost CONAN_PKG::fmt)
target_link_libraries(cppecho PRIVATE CONAN_PKG::log4cplus CONAN_PKG::Boost CONAN_PKG::fmt)

add_test(NAME all COMMAND testrunner)

#Copy default config to the output dir
configure_file(config/logger.cfg bin/logger.cfg COPYONLY)
