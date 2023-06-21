
project(SktAssist LANGUAGES C)

set(TARGET_NAME sktassist)
add_executable(${TARGET_NAME} "")

file(GLOB ASSIST_FILES 
    ${SRC_DIR}/*.c ${INC_DIR}/*.h)
target_sources(
    ${TARGET_NAME} PRIVATE
    ${ASSIST_FILES}
)
target_include_directories(
    ${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
)
target_link_libraries(
    ${TARGET_NAME} PRIVATE
    server clog ds
)


