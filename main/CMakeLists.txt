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

set(FILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/files")
# 嵌入的文件
set(EMBEDDED_BIN_FILES
    "${FILE_DIR}/frame_00.bin"
    "${FILE_DIR}/frame_01.bin"
    "${FILE_DIR}/frame_02.bin"
    "${FILE_DIR}/frame_03.bin"
    "${FILE_DIR}/frame_04.bin"
    "${FILE_DIR}/frame_05.bin"
    "${FILE_DIR}/frame_06.bin"
    "${FILE_DIR}/frame_07.bin"
    "${FILE_DIR}/frame_08.bin"
    "${FILE_DIR}/frame_09.bin"
    "${FILE_DIR}/frame_10.bin"
    "${FILE_DIR}/frame_11.bin"
    "${FILE_DIR}/frame_12.bin"
)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    EMBED_FILES ${EMBEDDED_BIN_FILES}
)