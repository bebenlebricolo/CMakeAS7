cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

find_package(GTest REQUIRED)
enable_testing()

add_executable(testCmStringUtils
    testCmStringUtils.cpp
    ${CMAKE_SOURCE_DIR}/Source/Utils/cmStringUtils.cxx
)

if(MSVC)
    # Handles both debug and Release modes using the generator expression
    set_property(TARGET testCmStringUtils PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


target_include_directories(testCmStringUtils PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/Utils
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(testCmStringUtils ${GTEST_LIBRARIES})

set_target_properties(testCmStringUtils
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Tests/CMakeLib/testCmStringUtils
)