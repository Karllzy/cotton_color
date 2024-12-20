//
// Created by zjc on 24-11-18.
//

#include "utils.h"


#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/mat.hpp>

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
    MIL_INT channel_num = MbufInquire(input_img,  M_SIZE_BAND, M_NULL);

    MbufAlloc2d(MilSystem, size_x, size_y, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &output_img);
    if(channel_num == 1) {
        MimArith(output_img, input_img, output_img, M_ADD);
        MimArith(output_img, 255.0, output_img, M_MULT_CONST);
    } else if(channel_num == 3) {
        MimConvert(input_img, output_img, M_RGB_TO_L);
        MimArith(output_img, M_NULL, output_img, M_NOT);
    } else {
        cout << "Unsupported channel number!" << endl;
    }
    return output_img;
}


wstring convert_to_wstring(const string& str) {
    return wstring(str.begin(), str.end());
}



void read_params_from_file(const std::string& filename, std::map<std::string, int>& params) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        // 去除行首和行尾的空白字符
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // 跳过空行和注释行
        if (line.empty() || line[0] == '#')
            continue;

        // 查找等号的位置
        size_t pos = line.find('=');
        if (pos == std::string::npos)
            continue; // 如果没有等号，跳过该行

        // 分割键和值，并去除空白字符
        std::string key = line.substr(0, pos);
        std::string value_str = line.substr(pos + 1);
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value_str.erase(0, value_str.find_first_not_of(" \t"));
        value_str.erase(value_str.find_last_not_of(" \t") + 1);

        // 将字符串转换为整数
        int value;
        std::istringstream iss(value_str);
        if (!(iss >> value)) {
            std::cerr << "键 " << key << " 的值无效: " << value_str << std::endl;
            continue;
        }

        // 将键值对添加到参数映射中
        params[key] = value;
    }
}

// 图片转换函数，输入4096*1024*3的图片，输出为(4096 / n_valves) * (1024 / n_merge_vertical) * 1
// Mat Mil2cvImage(MIL_ID &input_image,Mat) {}