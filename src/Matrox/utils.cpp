//
// Created by zjc on 24-11-18.
//

#include "utils.h"


#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

/**
 * Convert Lab values from Photoshop range to OpenCV range.
 *
 * @param lab_ps A vector of Lab values in Photoshop range [L (0-100), a (-128 to 127), b (-128 to 127)].
 * @return A vector of Lab values in OpenCV range [L (0-255), a (0-255), b (0-255)].
 */
vector<int> psLabToOpenCVLab(const vector<int>& lab_ps) {
    int l_ps = lab_ps[0];
    int a_ps = lab_ps[1];
    int b_ps = lab_ps[2];

    // Conversion formulas
    int l_cv = round((l_ps / 100.0) * 255.0);  // Scale L from 0-100 to 0-255
    int a_cv = round(((a_ps + 128.0) / 255.0) * 255.0);  // Shift and scale a
    int b_cv = round(((b_ps + 128.0) / 255.0) * 255.0);  // Shift and scale b

    return {l_cv, a_cv, b_cv};
}

/**
 * Convert Lab values from OpenCV range to Photoshop range.
 *
 * @param lab_cv A vector of Lab values in OpenCV range [L (0-255), a (0-255), b (0-255)].
 * @return A vector of Lab values in Photoshop range [L (0-100), a (-128 to 127), b (-128 to 127)].
 */
vector<int> opencvLabToPsLab(const vector<int>& lab_cv) {
    int l_cv = lab_cv[0];
    int a_cv = lab_cv[1];
    int b_cv = lab_cv[2];

    // Conversion formulas
    int l_ps = round((l_cv / 255.0) * 100.0);  // Scale L from 0-255 to 0-100
    int a_ps = round((a_cv / 255.0) * 255.0 - 128.0);  // Scale and shift a
    int b_ps = round((b_cv / 255.0) * 255.0 - 128.0);  // Scale and shift b

    return {l_ps, a_ps, b_ps};
}

MIL_ID convert_to_uint8(MIL_ID input_img) {
    MIL_ID output_img;
    MIL_ID MilSystem = MbufInquire(input_img, M_OWNER_SYSTEM, M_NULL);
    MIL_INT size_x = MbufInquire(input_img, M_SIZE_X, M_NULL);
    MIL_INT size_y = MbufInquire(input_img, M_SIZE_Y, M_NULL);
    MbufAlloc2d(MilSystem, size_x, size_y, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &output_img);
    MimArith(output_img, input_img, output_img, M_ADD);
    MimArith(output_img, 255.0, output_img, M_MULT_CONST);
    return output_img;
}

// 图片转换函数，输入4096*1024*3的图片，输出为(4096 / n_valves) * (1024 / n_merge_vertical) * 1
