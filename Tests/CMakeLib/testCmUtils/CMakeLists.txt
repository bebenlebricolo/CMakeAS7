cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

find_package(GTest REQUIRED)
enable_testing()

add_executable(testCmStringUtils
    testCmStringUtils.cpp
)

target_include_directories(testCmStringUtils PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/Utils
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(testCmStringUtils cmutils ${GTEST_LIBRARIES})

set_target_properties(testCmStringUtils
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Tests/CMakeLib/testCmStringUtils
)