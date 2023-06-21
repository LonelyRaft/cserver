
project(clog LANGUAGES C)

set(CLOG_NAME clog)
add_library(
    ${CLOG_NAME} SHARED
    "readme.md"
)

file(GLOB CLOG_FLES 
    ${SRC_DIR}/*.c ${INC_DIR}/*.h)
target_sources(
    ${CLOG_NAME} PRIVATE
    ${CLOG_FLES}
)
target_include_directories(
    ${CLOG_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
)

