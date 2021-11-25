cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(testsAtmelStudioComponents CXX)
find_package(GTest REQUIRED)
set(CMAKE_CXX_STANDARD 17 )

####### Atmel Studio 7 toolchains tests

add_executable(testAtmelStudioTools
    testCmAvrGccCompiler.cpp
)

target_include_directories(testAtmelStudioTools PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/Utils
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/Compiler/Options
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/AS7Toolchains
)

if (WIN32)
    CMAKE_SET_TARGET_FOLDER(testAtmelStudioTools "Tests")
    target_compile_options(testAtmelStudioTools PRIVATE "/W4")
endif()

if(WIN32)
    target_link_libraries(testAtmelStudioTools AtmelStudio7Generators cmutils gtest )
else()
    target_link_libraries(testAtmelStudioTools AtmelStudio7Generators cmutils gtest pthread)
endif()

set_target_properties(testAtmelStudioTools
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Tests/CMakeLib/testAtmelStudioTools
)


####### AS7 Device resolver tests

add_executable(testAS7DeviceResolver
    testAS7DeviceResolver.cpp
)

target_include_directories(testAS7DeviceResolver PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/Utils
    ${CMAKE_SOURCE_DIR}/Source/AtmelStudio7Generators/AS7Toolchains
)

if (WIN32)
    CMAKE_SET_TARGET_FOLDER(testAS7DeviceResolver "Tests")
    target_compile_options(testAS7DeviceResolver PRIVATE "/W4")
endif()


if(WIN32)
    target_link_libraries(testAS7DeviceResolver AtmelStudio7Generators cmutils gtest)
else()
    target_link_libraries(testAS7DeviceResolver AtmelStudio7Generators cmutils gtest pthread)
endif()


set_target_properties(testAS7DeviceResolver
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Tests/CMakeLib/testAS7DeviceResolver
)