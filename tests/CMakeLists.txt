cmake_minimum_required(VERSION 3.10)
project(cfl_tests C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/unity/src
)

# Unity test framework source
set(UNITY_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/unity/src/unity.c
)

# CFL library sources
set(CFL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/cfl.c
)

# Helper function to add test executables
function(add_cfl_test test_name test_source)
    add_executable(${test_name} ${test_source} ${CFL_SRC} ${UNITY_SRC})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Enable testing
enable_testing()

# Add unit tests
add_cfl_test(test_cfl_crc unit/test_cfl_crc.c)
add_cfl_test(test_cfl_header unit/test_cfl_header.c)
add_cfl_test(test_cfl_serialization unit/test_cfl_serialization.c)
add_cfl_test(test_cfl_validation unit/test_cfl_validation.c)
add_cfl_test(test_cfl_builders unit/test_cfl_builders.c)

# Add custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS
        test_cfl_crc
        test_cfl_header
        test_cfl_serialization
        test_cfl_validation
        test_cfl_builders
    COMMENT "Running all CFL unit tests"
)

# Add custom target for verbose test output
add_custom_target(run_tests_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS
        test_cfl_crc
        test_cfl_header
        test_cfl_serialization
        test_cfl_validation
        test_cfl_builders
    COMMENT "Running all CFL unit tests (verbose)"
)

# Coverage support (optional, for GCC/Clang)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -O0)
        add_link_options(--coverage)
    endif()
endif()
