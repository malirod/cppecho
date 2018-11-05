set(LIB_NAME cppecho)
set(APP_NAME echosrv)

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
add_library(rms::${LIB_NAME} ALIAS ${LIB_NAME})
add_sanitizers(${LIB_NAME})

target_include_directories(${LIB_NAME} PUBLIC src)
target_compile_features(${LIB_NAME} PRIVATE cxx_std_14)
target_link_libraries(${LIB_NAME} PUBLIC rms::flatasync)

add_executable(${APP_NAME} "src/core/lifecycle.cc")
target_compile_features(${APP_NAME} PRIVATE cxx_std_14)
add_sanitizers(${APP_NAME})
list(APPEND LCOV_REMOVE_PATTERNS "'*src/core/lifecycle.cc'")
target_link_libraries(${APP_NAME} PRIVATE ${LIB_NAME})

if (BUILD_TESTING)
    add_coverage(${LIB_NAME})
    add_coverage(${APP_NAME})

    set(TEST_LIB_NAME "${LIB_NAME}_test")

    set(TEST_SRC_LIST
        "test/core/engine_test.cc"
        "test/core/general_error_test.cc")

    add_library(${TEST_LIB_NAME} OBJECT ${TEST_SRC_LIST})
    add_library(rms::${TEST_LIB_NAME} ALIAS ${TEST_LIB_NAME})

    set_property(GLOBAL APPEND PROPERTY test_targets rms::${TEST_LIB_NAME})

    add_sanitizers(${TEST_LIB_NAME})
    list(APPEND LCOV_REMOVE_PATTERNS "'*test/*'")
    add_coverage(${TEST_LIB_NAME})

    target_include_directories(${TEST_LIB_NAME} PRIVATE test)
    target_compile_features(${TEST_LIB_NAME} PRIVATE cxx_std_14)
    target_link_libraries(${TEST_LIB_NAME} PUBLIC rms::${LIB_NAME} CONAN_PKG::gtest)
endif()