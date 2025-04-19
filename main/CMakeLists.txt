set(INCLUDE_DIRS "." "board" "service" "display")

# 收集 .cpp 源文件
file(GLOB_RECURSE BOARD_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/board/*.cpp)
file(GLOB_RECURSE SERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/service/*.cpp)
file(GLOB_RECURSE DISPLAY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/display/*.cpp)

# 合并所有源码文件
set(SOURCES
    main.cpp
    ${BOARD_SOURCES}
    ${SERVICE_SOURCES}
    ${DISPLAY_SOURCES}
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
)