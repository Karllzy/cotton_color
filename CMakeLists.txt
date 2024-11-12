cmake_minimum_required(VERSION 3.29)
project(cotton_color)


set(CMAKE_CXX_STANDARD 17)

# 设置 OpenCV 路径
if(DEFINED ENV{OpenCV_DIR})
    set(OpenCV_DIR ENV{OpenCV_DIR})
    message(STATUS "OpenCV_DIR set from environment variable: ${OpenCV_DIR}")
else()
    message(FATAL_ERROR "OpenCV_DIR is not set. Please set the OpenCV_DIR environment variable.")
endif()

# 找到 OpenCV 包
find_package(OpenCV REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)
# 包含头文件路径
include_directories(${OpenCV_INCLUDE_DIRS})

# 添加可执行文件
add_executable(cotton_color cotton_color.cpp)
# 链接 OpenCV 库
target_link_libraries(cotton_color ${OpenCV_LIBS}  comdlg32)


# 添加可执行文件
add_executable(cotton_color2 cotton_color2.cpp)
# 链接 OpenCV 库
target_link_libraries(cotton_color2 ${OpenCV_LIBS}  comdlg32)