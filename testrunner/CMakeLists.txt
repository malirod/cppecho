if (BUILD_TESTING)
  set(APP_NAME testrunner)

  add_executable(${APP_NAME} "src/testrunner.cc")
  target_link_libraries(${APP_NAME} PRIVATE -Wl,--whole-archive ${COMMON_NAMESPACE}::testflatasync ${COMMON_NAMESPACE}::testbasecppecho -Wl,--no-whole-archive CONAN_PKG::gtest)
  target_compile_options(${APP_NAME} PRIVATE ${COMMON_CPP_FLAGS})

  add_sanitizers(${APP_NAME})

  add_coverage(${APP_NAME})
endif()
