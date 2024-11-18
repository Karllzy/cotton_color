//
// Created by zjc on 24-11-12.
//

#ifndef COLOR_RANGE_H
#define COLOR_RANGE_H

#include <mil.h>
#include <map>
#include <string>

void lab_process(MIL_ID& inputImage, MIL_ID& outputImageLab, const std::map<std::string, int>& params);
// 用法
// std::map<std::string, int> params;
// params["saturation_threshold"] = 150;

void hsv_process(MIL_ID& inputImage, MIL_ID& outputImageHSV, const std::map<std::string, int>& params);
// 用法
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
//




#endif //COLOR_RANGE_H
