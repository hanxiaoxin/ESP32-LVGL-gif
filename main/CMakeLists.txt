# 设置包含路径
set(INCLUDE_DIRS "." "board" "service" "display" "assets")

# 匹配 main/board 里的所有 cpp（递归子目录）
file(GLOB_RECURSE ASSETS_SOURCES "assets/*.cpp")
file(GLOB_RECURSE BOARD_SOURCES "board/*.cpp")
file(GLOB_RECURSE SERVICE_SOURCES "service/*.cpp")
file(GLOB_RECURSE DISPLAY_SOURCES "display/*.cpp")


# 设置所有源文件
set(SOURCES
    ${ASSETS_SOURCES}
    utils.h
    main.cpp
    application.cpp
    background_task.cpp
    ${BOARD_SOURCES}
    ${SERVICE_SOURCES}
    ${DISPLAY_SOURCES}
)

set(FILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/files/")
set(FRAMES_DIR "${FILE_DIR}/frames")
file(GLOB EMBEDDED_BIN_FILES "${FRAMES_DIR}/frame_*.bin")

# 嵌入的文件
set(EMBEDDED_BIN_FILES
    ${EMBEDDED_BIN_FILES}
    "${FILE_DIR}/logo.bin"
)


idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    EMBED_FILES ${EMBEDDED_BIN_FILES}
)