# Setup include-what-you-use
find_program(IWYU_EXE NAMES "include-what-you-use" PATHS /usr/local/bin)
find_program(IWYU_SCRIPT NAMES "iwyu_tool.py" PATHS /usr/local/bin)
if(NOT IWYU_EXE OR NOT IWYU_SCRIPT)
    message(WARNING "Could not find the program(s) include-what-you-use or iwyu_tool.py")
else()
    message(STATUS "Found include-what-you-use: ${IWYU_EXE}")
    message(STATUS "Found iwyu_tool.py: ${IWYU_SCRIPT}")
    add_custom_target(iwyu
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/iwyu/run_iwyu.sh ${IWYU_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM)
endif()
