
# 测试用例 1: template_color_range
add_executable(test_color_range
        ${CMAKE_CURRENT_SOURCE_DIR}/test_color_range.cpp
)

# 链接 Matrox 模块和依赖库
target_link_libraries(test_color_range Matrox ${OpenCV_LIBS} ${MIL_LIBS})

# 测试用例 2: template_template_matching
add_executable(test_template_matching
        ${CMAKE_CURRENT_SOURCE_DIR}/test_template_matching.cpp
)

# 链接 Matrox 模块和依赖库
target_link_libraries(test_template_matching Matrox ${OpenCV_LIBS} ${MIL_LIBS})


add_executable(test_mask
        ${CMAKE_CURRENT_SOURCE_DIR}/test_mask.cpp
)

# 链接 Matrox 模块和依赖库
target_link_libraries(test_mask Matrox ${OpenCV_LIBS} ${MIL_LIBS})