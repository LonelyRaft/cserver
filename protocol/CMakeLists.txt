
project(Protocols LANGUAGES C)

set(TARGET_NAME protocol)
add_library(${TARGET_NAME} SHARED "")

file(GLOB PROTOCOL_FILES 
    ${SRC_DIR}/*.c ${INC_DIR}/*.h)
target_sources(
    ${TARGET_NAME} PRIVATE
    ${PROTOCOL_FILES}
)
target_include_directories(
    ${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
)
