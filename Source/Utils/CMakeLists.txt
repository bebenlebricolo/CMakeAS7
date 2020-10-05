set(UTILS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/cmEncoding.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/cmEncoding.h
)

add_library(cmutils SHARED
    ${UTILS_SRCS}
)