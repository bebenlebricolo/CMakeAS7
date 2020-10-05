set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cmAtmelStudio7TargetGenerator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmLocalAtmelStudio7Generator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmGlobalAtmelStudio7Generator.cxx
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmAtmelStudio7TargetGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmLocalAtmelStudio7Generator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmGlobalAtmelStudio7Generator.h
)

add_library(atmelstudio7generators
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(atmelstudio7generators PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(atmelstudio7generators PUBLIC
    pugixml
    cmutils
)
