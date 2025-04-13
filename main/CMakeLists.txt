set(INCLUDE_DIRS "." "lib" "service")

# 收集 .cpp 源文件
file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp)
file(GLOB SERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/service/*.cpp)

# 合并所有源码文件
set(SOURCES
    main.cpp
    ${LIB_SOURCES}
    ${SERVICE_SOURCES}
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
)