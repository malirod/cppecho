set(LIB_NAME basecppecho)
set(APP_NAME cppecho)

set(SRC_LIST
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
    "src/core/startup_config.cc"
    "src/core/startup_config.h")

add_library(${LIB_NAME} ${SRC_LIST})
add_library(${COMMON_NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})
add_sanitizers(${LIB_NAME})
add_coverage(${LIB_NAME})
target_include_directories(${LIB_NAME} PUBLIC src)
target_compile_options(${LIB_NAME} PRIVATE ${COMMON_CPP_FLAGS})
target_link_libraries(${LIB_NAME} PUBLIC ${COMMON_NAMESPACE}::flatasync)

add_executable(${APP_NAME} "src/core/lifecycle.cc")
target_compile_options(${APP_NAME} PRIVATE ${COMMON_CPP_FLAGS})
add_sanitizers(${APP_NAME})
add_coverage(${APP_NAME})
target_link_libraries(${APP_NAME} PRIVATE ${LIB_NAME})

if (BUILD_TESTING)
    set(TEST_LIB_NAME "test${LIB_NAME}")

    set(TEST_SRC_LIST
        "test/core/engine_test.cc"
        "test/core/general_error_test.cc")

    add_library(${TEST_LIB_NAME} ${TEST_SRC_LIST})
    add_library(${COMMON_NAMESPACE}::${TEST_LIB_NAME} ALIAS ${TEST_LIB_NAME})

    add_sanitizers(${TEST_LIB_NAME})
    add_coverage(${TEST_LIB_NAME})

    target_include_directories(${TEST_LIB_NAME} PRIVATE test)
    target_compile_options(${TEST_LIB_NAME} PRIVATE ${COMMON_CPP_FLAGS})
    target_link_libraries(${TEST_LIB_NAME} PUBLIC ${COMMON_NAMESPACE}::${LIB_NAME} ${COMMON_NAMESPACE}::testflatasync CONAN_PKG::gtest)
endif()