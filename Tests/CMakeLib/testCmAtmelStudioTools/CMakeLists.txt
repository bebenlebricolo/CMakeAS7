cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(testAtmelStudioTools CXX)
find_package(GTest REQUIRED)

####### Atmel Studio 7 toolchains tests

add_executable(testAtmelStudioTools
    testCmAvrGccCompiler.cpp
)

target_include_directories(testAtmelStudioTools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Source/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Source/AtmelStudio7Generators/Compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Source/AtmelStudio7Generators/Compiler/Options
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Source/AtmelStudio7Generators/AS7Toolchains
)

if (WIN32)
    CMAKE_SET_TARGET_FOLDER(testAtmelStudioTools "Tests")
    target_compile_options(testAtmelStudioTools PRIVATE "/W4")
endif()

target_link_libraries(testAtmelStudioTools AtmelStudio7Generators cmutils gtest )

set_target_properties(testAtmelStudioTools
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Tests/CMakeLib/testAtmelStudioTools
)


####### AS7 Device resolver tests

add_executable(testAS7DeviceResolver
    testAS7DeviceResolver.cpp
)

target_include_directories(testAS7DeviceResolver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Source/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Source/AtmelStudio7Generators/AS7Toolchains
)

if (WIN32)
    CMAKE_SET_TARGET_FOLDER(testAS7DeviceResolver "Tests")
    target_compile_options(testAS7DeviceResolver PRIVATE "/W4")
endif()


target_link_libraries(testAS7DeviceResolver AtmelStudio7Generators cmutils gtest)

set_target_properties(testAS7DeviceResolver
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Tests/CMakeLib/testAS7DeviceResolver
)