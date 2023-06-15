
project(clog LANGUAGES C)

set(CLOG_NAME clog)
add_library(
    ${CLOG_NAME} SHARED
    "readme.md"
)

file(GLOB CLOG_SRCS *.c *.h)
target_sources(
    ${CLOG_NAME} PRIVATE
    ${CLOG_SRCS}
)
target_include_directories(
    ${CLOG_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

