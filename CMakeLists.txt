cmake_minimum_required(VERSION 3.5)

project(SktAssist LANGUAGES C)

set(INC_DIR inc)
set(SRC_DIR src)

add_subdirectory(clog)
add_subdirectory(dsdef)
add_subdirectory(server)
add_subdirectory(protocol)

set(TARGET_NAME sktassist)
add_executable(${TARGET_NAME} "")

file(GLOB ASSIST_FILES 
    assist/${SRC_DIR}/*.c 
    assist/${INC_DIR}/*.h)
message(STATUS ${ASSIST_FILES})
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

