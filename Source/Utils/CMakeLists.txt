set(UTILS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmEncoding.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmEncoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmStringUtils.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmStringUtils.h
)

add_library(cmutils STATIC
    ${UTILS_SRCS}
)

target_include_directories(cmutils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)