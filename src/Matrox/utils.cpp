//
// Created by zjc on 24-11-18.
//

#include "utils.h"

#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/core/mat.hpp>
#include "Mil.h"
#include <opencv2/opencv.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;
using namespace cv;

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



// 函数：从配置文件中读取参数
std::unordered_map<std::string, int> loadConfig(const std::string& filename){
    unordered_map<string, int> config;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        // 跳过空行和注释行
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        // 删除行尾的空格
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        // 查找 '=' 分隔符的位置
        size_t pos = line.find('=');
        if (pos == string::npos) {
            continue; // 没有 '='，跳过此行
        }

        string key = line.substr(0, pos);  // 获取参数名
        string valueStr = line.substr(pos + 1);  // 获取参数值

        // 删除键和值两端的空格
        key.erase(key.find_last_not_of(" \t\n\r") + 1);
        valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);

        // 尝试将参数值转换为整数
        int value;
        stringstream(valueStr) >> value;

        config[key] = value;  // 将参数存入 map
    }

    return config;
}

// 图片转换函数，输入4096*1024*3的图片，输出为(4096 / n_valves) * (1024 / n_merge_vertical) * 1

Mat mil2mat(const MIL_ID mil_img) {
    // 获取 MIL 图像的宽度、高度和通道数
    MIL_INT width, height, channels, bitDepth;

    MbufInquire(mil_img, M_SIZE_X, &width);
    MbufInquire(mil_img, M_SIZE_Y, &height);
    MbufInquire(mil_img, M_SIZE_BAND, &channels);
    MbufInquire(mil_img, M_SIZE_BIT, &bitDepth);

    if (channels == 1) {
        // 单通道图像，直接读取整个缓冲区
        Mat grayImage(height, width, CV_8UC1);
        if (bitDepth == 1) {
            MIL_ID temp_img;
            temp_img = convert_to_uint8(mil_img);
            MbufGet(temp_img, grayImage.data);
            MbufFree(temp_img);
        } else {
            MbufGet(mil_img, grayImage.data);
        }
        return grayImage;
    }
    if (channels == 3) {
        // 多通道图像，分通道读取
        MIL_ID redChannel, greenChannel, blueChannel;
        MbufAlloc2d(M_DEFAULT, width, height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &redChannel);
        MbufAlloc2d(M_DEFAULT, width, height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &greenChannel);
        MbufAlloc2d(M_DEFAULT, width, height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &blueChannel);

        // 将 MIL 图像的各个通道复制到单通道缓冲区
        MbufCopyColor(mil_img, redChannel, M_RED);
        MbufCopyColor(mil_img, greenChannel, M_GREEN);
        MbufCopyColor(mil_img, blueChannel, M_BLUE);

        // 分别读取每个通道的数据
        Mat redMat(height, width, CV_8UC1);
        Mat greenMat(height, width, CV_8UC1);
        Mat blueMat(height, width, CV_8UC1);
        MbufGet(redChannel, redMat.data);
        MbufGet(greenChannel, greenMat.data);
        MbufGet(blueChannel, blueMat.data);
        // 释放通道缓冲区
        MbufFree(redChannel);
        MbufFree(greenChannel);
        MbufFree(blueChannel);

        // 合并通道
        std::vector<Mat> bgrChannels = {blueMat, greenMat, redMat};
        Mat colorImage;
        cv::merge(bgrChannels, colorImage);

        return colorImage;
    }
    // 不支持的通道数
    std::cerr << "[Error] Unsupported number of channels: " << channels << std::endl;
    return Mat();
}


#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

void displayCombinedResults(const std::vector<cv::Mat>& images, const std::string& windowName) {
    // Get the screen resolution (you can adjust this if needed, this is for a typical screen)
    int screen_width = round(GetSystemMetrics(SM_CXSCREEN) * 0.4);
    int screen_height = round(GetSystemMetrics(SM_CYSCREEN) * 0.4);

    // First, we need to find the maximum width and height among the input images
    int max_width = 0;
    int max_height = 0;

    // Loop over all images to get the maximum dimensions
    for (const auto& img : images) {
        if (!img.empty()) {
            max_width = max(max_width, img.cols);
            max_height = max(max_height, img.rows);
        }
    }

    // Resize all images to a reasonable scale to fit the screen
    float scale_factor = min((float)screen_width / max_width, (float)screen_height / max_height);

    // If the images are already too large, reduce the scale factor
    if (scale_factor > 1.0) scale_factor = 1.0;

    std::vector<cv::Mat> resized_images;
    for (const auto& img : images) {
        if (!img.empty()) {
            cv::Mat resized_img;
            // Resize each image based on the scale factor
            cv::resize(img, resized_img, cv::Size(), scale_factor, scale_factor);
            resized_images.push_back(resized_img);
        }
    }

    // Calculate the grid size for displaying images
    int num_images = resized_images.size();
    int num_cols = std::ceil(std::sqrt(num_images)); // Trying to make a square-like grid
    int num_rows = std::ceil((float)num_images / num_cols);

    std::vector<cv::Mat> rows;

    for (int i = 0; i < num_rows; ++i) {
        std::vector<cv::Mat> row_images;

        for (int j = 0; j < num_cols; ++j) {
            int index = i * num_cols + j;
            if (index < resized_images.size()) {
                row_images.push_back(resized_images[index]);
            } else {
                // If there's no image, create an empty placeholder (black) image
                row_images.push_back(cv::Mat::zeros(resized_images[0].rows, resized_images[0].cols, resized_images[0].type()));
            }
        }

        // Concatenate horizontally to create a single row
        cv::Mat row;
        cv::hconcat(row_images, row);
        rows.push_back(row);
    }

    // Concatenate all rows vertically to create the final grid
    cv::Mat final_result;
    cv::vconcat(rows, final_result);

    // Set the window properties to allow resizing
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, screen_width, screen_height); // Resize window to screen size

    // Display the combined result
    cv::imshow(windowName, final_result);

    // Wait for the user to press a key or the window to close
    cv::waitKey(0);
}
