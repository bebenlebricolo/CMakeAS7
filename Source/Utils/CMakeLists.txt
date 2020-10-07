set(UTILS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmEncoding.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmEncoding.h
)

add_library(cmutils SHARED
    ${UTILS_SRCS}
)

target_include_directories(cmutils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)