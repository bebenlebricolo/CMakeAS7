cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

find_package(GTest REQUIRED)
enable_testing()

add_executable(testAtmelStudioTools
    testCmAvrGccCompiler.cpp
)

# Enable unit testing features in codebase
target_compile_definitions(testAtmelStudioTools
    PUBLIC -DUNIT_TESTING
)

if(MSVC)
    # Handles both debug and Release modes using the generator expression
    set_property(TARGET testAtmelStudioTools PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


target_include_directories(testAtmelStudioTools PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/Utils
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Options
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(testAtmelStudioTools atmelstudio7generators cmutils ${GTEST_LIBRARIES})

set_target_properties(testAtmelStudioTools
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Tests/CMakeLib/testAtmelStudioTools
)