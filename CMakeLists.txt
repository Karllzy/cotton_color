cmake_minimum_required(VERSION 3.10)
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
# 包含 OpenCV 头文件路径
include_directories(${OpenCV_INCLUDE_DIRS})

# 设置 Qt 路径
if(DEFINED ENV{Qt6_DIR})
    set(Qt6_DIR ENV{Qt6_DIR})
    message(STATUS "Qt6_DIR set from environment variable: ${Qt6_DIR}")
else()
    message(FATAL_ERROR "Qt6_DIR is not set. Please set the Qt6_DIR environment variable.")
endif()
# 查找 Qt6 包
find_package(Qt6 REQUIRED COMPONENTS Widgets)
# 自动处理 MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


# 添加可执行文件 cotton_color
add_executable(cotton_color cotton_color.cpp)
# 链接 OpenCV 和 Qt 库
target_link_libraries(cotton_color Qt6::Widgets ${OpenCV_LIBS} comdlg32)

# 添加可执行文件 cotton_color
add_executable(cotton_range Matrox/color_range.cpp)
# 链接 OpenCV 和 Qt 库
target_link_libraries(cotton_range Qt6::Widgets ${OpenCV_LIBS})

# 添加可执行文件 cotton_color2
add_executable(cotton_color2 cotton_color2.cpp)
# 链接 OpenCV 和 Qt 库
target_link_libraries(cotton_color2 Qt6::Widgets ${OpenCV_LIBS})
add_executable(color_matching Matrox/template_matching.cpp
        Matrox/onnx_running.cpp
        Matrox/onnx_running.h
        Matrox/ui.cpp)

add_executable(ui Matrox/ui.cpp)
target_link_libraries(ui Qt6::Widgets)