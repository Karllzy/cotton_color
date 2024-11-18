cmake_minimum_required(VERSION 3.29)
project(cotton_color)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DUNICODE -D_UNICODE)

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

#配置mil库
include_directories(E:/QTexamble/matrox/Include)
# 添加 MIL 库的库文件路径
link_directories(E:/QTexamble/matrox/LIB)
file(GLOB MIL_LIBS E:/QTexamble/matrox/LIB/*.lib)

# 添加可执行文件 cotton_color
add_executable(cotton_color cotton_color.cpp)
# 链接 OpenCV 和 Qt 库
target_link_libraries(cotton_color Qt6::Widgets ${OpenCV_LIBS} comdlg32)

# 添加可执行文件 cotton_color
add_executable(cotton_range Matrox/color_range.cpp
        Matrox/color_range.h Matrox/utils.h Matrox/utils.cpp)
# 链接 OpenCV 和 Qt 库
target_link_libraries(cotton_range Qt6::Widgets ${OpenCV_LIBS} ${MIL_LIBS})

# 添加可执行文件 cotton_color2
add_executable(cotton_color2 cotton_color2.cpp)
# 链接 OpenCV 和 Qt 库
target_link_libraries(cotton_color2 Qt6::Widgets ${OpenCV_LIBS} ${MIL_LIBS})

add_executable(color_matching Matrox/template_matching.cpp
        Matrox/utils.cpp
        Matrox/utils.h)
target_link_libraries(color_matching Qt6::Widgets ${OpenCV_LIBS} ${MIL_LIBS})


add_executable(ui Matrox/ui.cpp)
target_link_libraries(ui Qt6::Widgets)


add_executable(onnx Matrox/onnx_running.cpp)
target_link_libraries(onnx Qt6::Widgets ${MIL_LIBS})