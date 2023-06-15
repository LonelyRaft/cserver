cmake_minimum_required(VERSION 3.5)

project(server LANGUAGES C)

add_subdirectory(clog)
add_subdirectory(dsdef)

set(TARGET_NAME server)
add_executable(${TARGET_NAME}
    ".gitignore"
    "readme.md"
)

file(GLOB SRC_LIST *.c *.h)
target_sources(
    ${TARGET_NAME} PRIVATE
    ${SRC_LIST}
)
target_link_libraries(
    ${TARGET_NAME} PRIVATE
    clog ds
)
message(${CMAKE_HOST_SYSTEM_NAME})
if(${CMAKE_HOST_SYSTEM_NAME} MATCHES Windows)
    target_link_libraries(
        ${TARGET_NAME} PRIVATE
        ws2_32
    )
endif()
