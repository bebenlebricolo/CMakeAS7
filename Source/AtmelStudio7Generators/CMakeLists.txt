set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cmAtmelStudio7TargetGenerator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmLocalAtmelStudio7Generator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmGlobalAtmelStudio7Generator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/cmAvrGccCompiler.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccOptimizationOption.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccDebugOption.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccCompilerOption.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/AS7Toolchains/AvrGCC8Toolchain.cxx
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmAtmelStudio7TargetGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmLocalAtmelStudio7Generator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmGlobalAtmelStudio7Generator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/cmAvrGccCompiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccCompilerOption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccOptimizationOption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccWarningOption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccLinkerOption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options/cmAvrGccDebugOption.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AS7Toolchains/AvrGCC8Toolchain.h
)

add_library(atmelstudio7generators
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(atmelstudio7generators PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/Compiler/Options
    ${CMAKE_CURRENT_SOURCE_DIR}/AS7Toolchains
)

target_link_libraries(atmelstudio7generators PUBLIC
    pugixml
    cmutils
)
