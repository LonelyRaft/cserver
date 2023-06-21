
project(server LANGUAGES C)

set(TARGET_NAME server)
add_library(${TARGET_NAME} SHARED "")

file(GLOB SERVER_FILES 
    ${SRC_DIR}/*.c ${INC_DIR}/*.h)
target_sources(
    ${TARGET_NAME} PRIVATE
    ${SERVER_FILES}
)
target_include_directories(
    ${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
)
target_link_libraries(
    ${TARGET_NAME} PRIVATE
    clog ds protocol
)
if(${CMAKE_HOST_SYSTEM_NAME} MATCHES Windows)
    target_link_libraries(
        ${TARGET_NAME} PRIVATE
        ws2_32
    )
endif()
