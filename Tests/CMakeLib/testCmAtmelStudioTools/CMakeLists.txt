cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(testsAtmelStudioComponents CXX)
#find_package(GTest REQUIRED)
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
else()
    target_compile_options(testAVR8GCCToolchain PRIVATE "-Werror -Wall -Wextra")
endif()

if(WIN32)
    target_link_libraries(testAtmelStudioTools AtmelStudio7Generators cmutils gtest )
else()
    target_link_libraries(testAtmelStudioTools AtmelStudio7Generators cmutils gtest pthread)
endif()


####### Test AVR8GCC toolchain

# We explicitely recompile AvrGCC8Toolchain in order to only draw it
add_executable(testAVR8GCCToolchain
    testAVR8GCCToolchain.cpp
    Stubs/StubCompilerModel.cpp
    Stubs/StubCompilerModel.h
)

target_include_directories(testAVR8GCCToolchain PUBLIC
    Stubs/
)

if (WIN32)
    CMAKE_SET_TARGET_FOLDER(testAVR8GCCToolchain "Tests")
    target_compile_options(testAVR8GCCToolchain PRIVATE "/W4")
else()
    target_compile_options(testAVR8GCCToolchain PRIVATE "-Werror -Wall -Wextra")
endif()

if(WIN32)
    target_link_libraries(testAVR8GCCToolchain AtmelStudio7Generators cmutils pugixml gtest )
else()
    target_link_libraries(testAVR8GCCToolchain AtmelStudio7Generators cmutils pugixml gtest pthread)
endif()


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
else()
    target_compile_options(testAS7DeviceResolver PRIVATE "-Werror -Wall -Wextra")
endif()



if(WIN32)
    target_link_libraries(testAS7DeviceResolver AtmelStudio7Generators cmutils gtest)
else()
    target_link_libraries(testAS7DeviceResolver AtmelStudio7Generators cmutils gtest pthread)
endif()
