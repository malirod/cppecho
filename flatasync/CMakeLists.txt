set(LIB_NAME flatasync)

set(SRC_LIST
    "src/core/alias.h"
    "src/core/async.cc"
    "src/core/async.h"
    "src/core/async_op_state.cc"
    "src/core/async_op_state.h"
    "src/core/async_proxy.cc"
    "src/core/async_proxy.h"
    "src/core/async_runner.cc"
    "src/core/async_runner.h"
    "src/core/coro_helper.cc"
    "src/core/coro_helper.h"
    "src/core/default_scheduler_accessor.cc"
    "src/core/default_scheduler_accessor.h"
    "src/core/iioservice.h"
    "src/core/ischeduler.h"
    "src/core/sequential_scheduler.cc"
    "src/core/sequential_scheduler.h"
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
    "src/util/static_string.h"
    "src/util/thread_util.cc"
    "src/util/thread_util.h"
    "src/util/type_traits.h")

add_library(${LIB_NAME} ${SRC_LIST})
add_library(${COMMON_NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

add_sanitizers(${LIB_NAME})
add_coverage(${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC src)
target_compile_options(${LIB_NAME} PRIVATE ${COMMON_CPP_FLAGS})

target_link_libraries(${LIB_NAME} PUBLIC CONAN_PKG::log4cplus CONAN_PKG::Boost CONAN_PKG::fmt)

if (BUILD_TESTING)
    set(TEST_LIB_NAME "test${LIB_NAME}")

    set(TEST_SRC_LIST
        "test/core/async_proxy_test.cc"
        "test/core/async_test.cc"
        "test/core/coro_helper_test.cc"
        "test/core/coroutine_test.cc"
        "test/core/helper.cc"
        "test/core/helper.h"
        "test/net/tcp_server_test.cc"
        "test/net/tcp_socket_test.cc"
        "test/util/enum_util_test.cc"
        "test/util/logger_test.cc"
        "test/util/rvo_test.cc"
        "test/util/scope_guard_test.cc"
        "test/util/singleton_test.cc"
        "test/util/static_string_test.cc")

    add_library(${TEST_LIB_NAME} ${TEST_SRC_LIST})
    add_library(${COMMON_NAMESPACE}::${TEST_LIB_NAME} ALIAS ${TEST_LIB_NAME})

    add_sanitizers(${TEST_LIB_NAME})

    add_coverage(${TEST_LIB_NAME})

    target_include_directories(${TEST_LIB_NAME} PUBLIC test)

    target_compile_options(${TEST_LIB_NAME} PRIVATE ${COMMON_CPP_FLAGS})

    target_link_libraries(${TEST_LIB_NAME} PUBLIC ${COMMON_NAMESPACE}::${LIB_NAME} CONAN_PKG::gtest)
endif()
