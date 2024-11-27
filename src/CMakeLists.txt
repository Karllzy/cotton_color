# 定义 Matrox 模块的库
add_library(Matrox
        Matrox/color_range.cpp
        Matrox/utils.cpp
        Matrox/template_matching.cpp
        Matrox/mask.cpp
)
# 头文件路径
target_include_directories(Matrox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# 链接依赖库
target_link_libraries(Matrox PUBLIC Qt6::Widgets ${OpenCV_LIBS} ${MIL_LIBS})


#定义CVDL模块的库
add_library(CVDL
       CVDL/OnnxRunner.cpp
)
# 头文件路径
target_include_directories(CVDL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# 链接依赖库
target_link_libraries(CVDL PUBLIC Qt6::Widgets Matrox ${OpenCV_LIBS} ${MIL_LIBS})