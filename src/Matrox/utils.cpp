//
// Created by zjc on 24-11-18.
//

#include "utils.h"


#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/mat.hpp>
#include "Mil.h"
#include <opencv2/opencv.hpp>
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

#include "mil.h"  // 包含 MIL 库的头文件

// cv::Mat milToMat(MIL_ID milImage, MIL_ID MilSystem) {
//     // 获取图像的宽度、高度和像素格式
//     int width, height, channels;
//     MbufInquire(milImage, M_SIZE_X, &width);  // 获取图像宽度
//     MbufInquire(milImage, M_SIZE_Y, &height); // 获取图像高度
//     MbufInquire(milImage, M_SIZE_BAND, &channels); // 获取图像通道数
//
//     // 为图像数据分配内存
//     byte* buf = new byte[width * height]();  // 初始化内存
//
//     // 如果是彩色图像，则转换为灰度图
//     if (channels > 1) {
//         MIL_ID grayscaleImage;
//         // 分配一个新的灰度图像缓冲区
//         MbufAlloc2d(MilSystem, width, height, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &grayscaleImage);
//         // 将彩色图像转换为灰度图
//         MimConvert(milImage, grayscaleImage, M_RGB_TO_L);
//         // 获取转换后的灰度图像数据
//         MbufGet(grayscaleImage, buf);
//         MbufFree(grayscaleImage);  // 释放灰度图像缓冲区
//     } else {
//         // 如果本身是灰度图像，直接获取数据
//         MbufGet(milImage, buf);
//     }
//
//     // 将数据封装成OpenCV Mat对象
//     cv::Mat cvMat(height, width, CV_8UC1, buf);
//     return cvMat;
// }
cv::Mat milToMat(MIL_ID milImage) {
    // 获取图像的宽度、高度和像素格式
    int width, height;
    MIL_INT pixelFormat;
    MbufInquire(milImage, M_SIZE_X, &width);  // 获取图像宽度
    MbufInquire(milImage, M_SIZE_Y, &height); // 获取图像高度
    MbufInquire(milImage, M_TYPE, &pixelFormat); // 获取图像像素格式

    // 判断像素格式并选择适当的数据类型
    // MIL_INT 和 M_UINT8 是典型的像素格式，OpenCV 处理不同的类型会有不同的实现
    cv::Mat matImage;
    if (pixelFormat == M_RGB)
    {
        // 如果是RGB图像
        matImage = cv::Mat(height, width, CV_8UC3);
    }
    else
    {// 如果是其他类型的图像，你需要根据实际格式进行调整
        std::cerr << "Unsupported MIL image format!" << std::endl;
        return matImage;
    }

    // 获取图像数据并复制到 OpenCV Mat
    MbufGet(milImage, matImage.data);  // 获取 MIL 图像数据并拷贝到 Mat 对象

    return matImage;
}
void processImage(cv::Mat& img) {
    // 1. 将图像从BGR转换到HSV色彩空间
    cv::Mat hsv;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

    // 2. 定义绿色的HSV色彩范围
    cv::Scalar lower_green(35, 40, 40); // 低阈值 (H, S, V)
    cv::Scalar upper_green(85, 255, 255); // 高阈值 (H, S, V)

    // 3. 根据绿色范围创建掩膜
    cv::Mat green_mask;
    cv::inRange(hsv, lower_green, upper_green, green_mask);

    // 4. 使用形态学操作去除噪点，增强绿色区域
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(green_mask, green_mask, cv::MORPH_CLOSE, kernel);

    // 5. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(green_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 6. 创建一个黑色背景图像
    cv::Mat result = cv::Mat::zeros(img.size(), img.type());

    // 7. 遍历轮廓，找到矩形框并将其内部填充为白色
    for (size_t i = 0; i < contours.size(); i++) {
        // 通过近似多边形来检测矩形
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.02, true);

        // 如果是四边形，认为是矩形
        if (approx.size() == 4) {
            // 计算矩形的bounding box
            cv::Rect rect = cv::boundingRect(approx);

            // 仅在绿色区域内将矩形框填充为白色
            cv::rectangle(result, rect, cv::Scalar(255, 255, 255), cv::FILLED);
        }
    }

    // 8. 显示结果图像
    cv::imshow("Processed Image", result);
    cv::waitKey(0);
}