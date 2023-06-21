
project(datastruct LANGUAGES C)

set(DS_NAME ds)
add_library(
    ${DS_NAME} SHARED
    "readme.md"
)

file(GLOB DS_FILES 
    ${SRC_DIR}/*.c ${INC_DIR}/*.h)
target_sources(
    ${DS_NAME} PRIVATE
    ${DS_FILES}
)
target_include_directories(
    ${DS_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)
