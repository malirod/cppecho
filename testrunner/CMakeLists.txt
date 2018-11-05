if (BUILD_TESTING)
    set(APP_NAME testrunner)

    add_executable(${APP_NAME} "src/testrunner.cc")

    get_property(_tests GLOBAL PROPERTY test_targets)
    list(REMOVE_DUPLICATES _tests)
    target_compile_features(${APP_NAME} PRIVATE cxx_std_14)

    target_link_libraries(${APP_NAME} PRIVATE ${_tests})

    add_sanitizers(${APP_NAME})
    add_coverage(${APP_NAME})
endif()