set(AS7_GENERATOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cmAtmelStudio7TargetGenerator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmLocalAtmelStudio7Generator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmGlobalAtmelStudio7Generator.cxx
)

set(AS7_GENERATOR_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmAtmelStudio7TargetGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmLocalAtmelStudio7Generator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmGlobalAtmelStudio7Generator.h
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(AS7Toolchains)
add_subdirectory(Compiler)

add_library(AtmelStudio7Generators
    ${AVR_GCC_COMPILER_SOURCES}
    ${AVR_GCC_COMPILER_HEADERS}

    ${AVR_GCC_COMPILER_OPTIONS_HEADERS}
    ${AVR_GCC_COMPILER_OPTIONS_SOURCES}

    ${AVR_GCC_TOOLCHAIN_SOURCES}
    ${AVR_GCC_TOOLCHAIN_HEADERS}

    ${AS7_GENERATOR_HEADERS}
    ${AS7_GENERATOR_SOURCES}
)

target_include_directories(AtmelStudio7Generators PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options
    ${CMAKE_CURRENT_SOURCE_DIR}/AS7Toolchains
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/AS7Toolchains PREFIX "Header Files\\AS7Toolchains" FILES ${AVR_GCC_TOOLCHAIN_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/AS7Toolchains PREFIX "Source Files\\AS7Toolchains" FILES ${AVR_GCC_TOOLCHAIN_SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Compiler PREFIX "Header Files\\Compiler" FILES ${AVR_GCC_COMPILER_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Compiler PREFIX "Source Files\\Compiler" FILES ${AVR_GCC_COMPILER_SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options PREFIX "Header Files\\Compiler\\Options" FILES ${AVR_GCC_COMPILER_OPTIONS_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options PREFIX "Source Files\\Compiler\\Options" FILES ${AVR_GCC_COMPILER_OPTIONS_SOURCES})

target_link_libraries(AtmelStudio7Generators PUBLIC
    pugixml
    cmutils
)
