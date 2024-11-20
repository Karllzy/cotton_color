# MIL库环境配置

include_directories(E:/QTexamble/matrox/Include)  
将路径修改为你的安装目录   .../Matrox Imaging/MIL/Include
# 添加 MIL 库的库文件路径
link_directories(E:/QTexamble/matrox/LIB)
file(GLOB MIL_LIBS E:/QTexamble/matrox/LIB/*.lib)
同理 将E:/QTexamble/matrox/LIB部分替换为安装目录下的.../Matrox Imaging/MIL/LIB   即可


## 鲜艳色彩检测功能

对应函数：

```c++
void lab_process(const MIL_ID& inputImage, MIL_ID& outputImageLab, const std::map<std::string, int>& params);
// 用法
// std::map<std::string, int> params;
// params["saturation_threshold"] = 150;
// params["saturation_denoising"] = 2;

void hsv_process(const MIL_ID& inputImage, MIL_ID& outputImageHSV, const std::map<std::string, int>& params);
// std::map<std::string, int> params;
// params["green_L_min"] = 68;
// params["green_L_max"] = 125;
// params["green_a_min"] = 101;
// params["green_a_max"] = 120;
// params["green_b_min"] = 130;
// params["green_b_max"] = 140;
//
// params["blue_L_min"] = 45;
// params["blue_L_max"] = 66;
// params["blue_a_min"] = 130;
// params["blue_a_max"] = 145;
// params["blue_b_min"] = 95;
// params["blue_b_max"] = 105;
//
// params["orange_L_min"] = 166;
// params["orange_L_max"] = 191;
// params["orange_a_min"] = 135;
// params["orange_a_max"] = 142;
// params["orange_b_min"] = 160;
// params["orange_b_max"] = 174;
//
// params["black_L_min"] = 0;
// params["black_L_max"] = 21;
// params["black_a_min"] = 127;
// params["black_a_max"] = 133;
// params["black_b_min"] = 126;
// params["black_b_max"] = 134;
//
// params["red_L_min"] = 71;
// params["red_L_max"] = 97;
// params["red_a_min"] = 143;
// params["red_a_max"] = 153;
// params["red_b_min"] = 33;
// params["red_b_max"] = 154;
//
// params["purple_L_min"] = 171;
// params["purple_L_max"] = 197;
// params["purple_a_min"] = 131;
// params["purple_a_max"] = 141;
// params["purple_b_min"] = 108;
// params["purple_b_max"] = 123;
// params["lab_denoising"] = 1;

```

|                   | hsv_denoising = 0 | hsv_denoising = 1 | hsv_denoising = 2                           |
| ----------------- | ----------------- | ----------------- | ------------------------------------------- |
| lab_denoising = 0 |                   |                   |                                             |
| lab_denoising = 1 |                   |                   | ![diguandai](./README.assets/diguandai.png) |
| lab_denoising = 2 |                   |                   |                                             |


这些是经过实验后的推荐参数：

|        | L_min | L_max | a_min | a_max | b_min | b_max |
| ------ | ----- | ----- | ----- | ----- | ----- | ----- |
| green  | 27    | 49    | -27   | -8    | 2     | 12    |
| blue   | 18    | 26    | 2     | 17    | -33   | -23   |
| orange | 65    | 75    | 7     | 14    | 32    | 46    |
| black  | 0     | 8     | -1    | 5     | -2    | 6     |
| red    | 28    | 38    | 15    | 25    | -95   | 26    |
| purple | 67    | 77    | 3     | 13    | -20   | -5    |

