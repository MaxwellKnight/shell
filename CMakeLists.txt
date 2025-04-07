cmake_minimum_required(VERSION 3.10)
project(shell C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(BUILD_DIR ${CMAKE_BINARY_DIR})

include_directories(${INCLUDE_DIR})

add_executable(shell
    ${SRC_DIR}/shell.c
    ${SRC_DIR}/main.c
)

enable_testing()

add_library(shell_obj OBJECT ${SRC_DIR}/shell.c)
set_target_properties(shell_obj PROPERTIES POSITION_INDEPENDENT_CODE 1)

add_executable(test_main ${TEST_DIR}/test_main.c)
target_sources(test_main PRIVATE $<TARGET_OBJECTS:shell_obj>)
add_test(NAME test_main COMMAND test_main)

add_executable(test_parse ${TEST_DIR}/test_parse.c)
target_sources(test_parse PRIVATE $<TARGET_OBJECTS:shell_obj>)
add_test(NAME test_parse COMMAND test_parse)

add_executable(test_history ${TEST_DIR}/test_history.c)
target_sources(test_history PRIVATE $<TARGET_OBJECTS:shell_obj>)
add_test(NAME test_history COMMAND test_history)

add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_main test_parse test_history
    COMMENT "Running all tests"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_DIR}
    COMMENT "Cleaning build directory"
)

install(TARGETS shell
    RUNTIME DESTINATION bin
)
