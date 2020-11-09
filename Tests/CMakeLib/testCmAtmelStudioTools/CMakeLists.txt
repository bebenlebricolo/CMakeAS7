cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

find_package(GTest REQUIRED)
enable_testing()

add_executable(testAtmelStudioTools
    testCmAvrGccCompiler.cpp
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Flags/cmAvrGccOptimizationFlag.cxx
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Flags/cmAvrGccOptimizationFlag.h
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Flags/cmAvrGccCompilerFlag.h
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Flags/cmAvrGccWarningFlag.h
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/cmAvrGccCompiler.cxx
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/cmAvrGccCompiler.h
)

if(MSVC)
    # Handles both debug and Release modes using the generator expression
    set_property(TARGET testAtmelStudioTools PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


target_include_directories(testAtmelStudioTools PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Flags
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(testAtmelStudioTools ${GTEST_LIBRARIES})

set_target_properties(testAtmelStudioTools
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Tests/CMakeLib/testAtmelStudioTools
)