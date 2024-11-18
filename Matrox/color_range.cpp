#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "utils.h"

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton_image\\174.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\diguandai.png")


// Global variables
MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;

// Optimized LabProcess function
void lab_process(const MIL_ID& inputImage, MIL_ID& outputImageLab, const std::map<std::string, int>& params)
{
    MIL_ID MilLabImage = M_NULL, MilLChannel = M_NULL, MilAChannel = M_NULL, MilBChannel = M_NULL;

    // Check number of bands
    MIL_INT NumBands = 0;
    MbufInquire(inputImage, M_SIZE_BAND, &NumBands);
    if (NumBands != 3)
    {
        printf("输入图像不是 3 通道图像，请提供彩色图像。\n");
        return;
    }

    // Inquire image properties once
    MIL_ID MilSystem = MbufInquire(inputImage, M_OWNER_SYSTEM, M_NULL);
    MIL_INT SizeX = MbufInquire(inputImage, M_SIZE_X, M_NULL);
    MIL_INT SizeY = MbufInquire(inputImage, M_SIZE_Y, M_NULL);

    // Allocate buffer for Lab image
    MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilLabImage);

    // Convert image from sRGB to Lab
    MimConvert(inputImage, MilLabImage, M_SRGB_TO_LAB);

    // Create child buffers for L, a, b channels
    MbufChildColor(MilLabImage, 0, &MilLChannel);
    MbufChildColor(MilLabImage, 1, &MilAChannel);
    MbufChildColor(MilLabImage, 2, &MilBChannel);

    // Allocate output image as 1-bit image
    MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &outputImageLab);
    MbufClear(outputImageLab, 0);  // Initialize to 0

    // Pre-allocate binary buffers as 1-bit images
    MIL_ID MilBinaryL = M_NULL, MilBinaryA = M_NULL, MilBinaryB = M_NULL, MilResultLab = M_NULL;
    MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &MilBinaryL);
    MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &MilBinaryA);
    MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &MilBinaryB);
    MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &MilResultLab);

    const std::vector<std::string> colors = {"green", "blue", "orange", "black", "red", "purple"};

    // Iterate over colors
    // 遍历颜色
    for (const auto& color : colors) {
        // 构建参数键
        std::string L_min_key = color + "_L_min";
        std::string L_max_key = color + "_L_max";
        std::string a_min_key = color + "_a_min";
        std::string a_max_key = color + "_a_max";
        std::string b_min_key = color + "_b_min";
        std::string b_max_key = color + "_b_max";

        // 获取参数值
        int L_min = params.at(L_min_key);
        int L_max = params.at(L_max_key);
        int a_min = params.at(a_min_key);
        int a_max = params.at(a_max_key);
        int b_min = params.at(b_min_key);
        int b_max = params.at(b_max_key);

        // 对每个通道进行二值化
        MimBinarize(MilLChannel, MilBinaryL, M_IN_RANGE, L_min, L_max);
        MimBinarize(MilAChannel, MilBinaryA, M_IN_RANGE, a_min, a_max);
        MimBinarize(MilBChannel, MilBinaryB, M_IN_RANGE, b_min, b_max);

        // 合并阈值结果
        MimArith(MilBinaryL, MilBinaryA, MilResultLab, M_AND);
        MimArith(MilResultLab, MilBinaryB, MilResultLab, M_AND);

        // 与输出图像合并
        MimArith(outputImageLab, MilResultLab, outputImageLab, M_OR);
    }

    // Free binary buffers
    MbufFree(MilBinaryL);
    MbufFree(MilBinaryA);
    MbufFree(MilBinaryB);
    MbufFree(MilResultLab);

    // Free resources
    MbufFree(MilLChannel);
    MbufFree(MilAChannel);
    MbufFree(MilBChannel);
    MbufFree(MilLabImage);
}

void hsv_process(const MIL_ID& inputImage, MIL_ID& outputImageHSV, const std::map<std::string, int>& params)
{
    MIL_ID MilHSVImage = M_NULL, MilHChannel = M_NULL, MilSChannel = M_NULL, MilVChannel = M_NULL;
    int saturationThreshold = params.at("saturation_threshold");
    // 检查输入图像的通道数
    MIL_INT NumBands = 0;
    MbufInquire(inputImage, M_SIZE_BAND, &NumBands);
    if (NumBands != 3)
    {
        printf("输入图像不是 3 通道图像，请提供彩色图像。\n");
        return;
    }

    // 分配用于存储 HSV 图像的缓冲区
    MbufAllocColor(MbufInquire(inputImage, M_OWNER_SYSTEM, M_NULL), 3,
        MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL),
        8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP,
        &MilHSVImage);

    // 将图像从 sRGB 转换到 HSV
    MimConvert(inputImage, MilHSVImage, M_RGB_TO_HSV);

    // 创建 HSV 通道的子缓冲区
    MbufChildColor(MilHSVImage, 0, &MilHChannel);
    MbufChildColor(MilHSVImage, 1, &MilSChannel);
    MbufChildColor(MilHSVImage, 2, &MilVChannel);

    // 分配输出图像缓冲区
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &outputImageHSV);

    // 对 S 通道进行阈值分割
    MimBinarize(MilSChannel, outputImageHSV, M_GREATER,
        saturationThreshold, M_NULL);

    // 释放资源
    MbufFree(MilHChannel);
    MbufFree(MilSChannel);
    MbufFree(MilVChannel);
    MbufFree(MilHSVImage);
}

void high_sat_detect(const MIL_ID& inputImage, MIL_ID& outputImage, const std::map<std::string, int>& params) {
    MIL_ID output_hsv=M_NULL, output_lab=M_NULL;

    hsv_process(inputImage, output_hsv, params);
    lab_process(inputImage, output_lab, params);

    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 1 + M_UNSIGNED,
        M_IMAGE + M_PROC, &outputImage);

    // 合并 Lab 和 HSV 的结果（取“或”运算）
    MimArith(output_hsv, output_lab, outputImage, M_OR);

    MbufFree(output_lab);
    MbufFree(output_hsv);
}

int main()
{
    // Initialize MIL application
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
        M_NULL);

    // Load input image
    MIL_ID MilImage = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);

    // Define color ranges
    std::map<std::string, int> params;
    params["green_L_min"] = 68;
    params["green_L_max"] = 125;
    params["green_a_min"] = 101;
    params["green_a_max"] = 120;
    params["green_b_min"] = 130;
    params["green_b_max"] = 140;

    params["blue_L_min"] = 45;
    params["blue_L_max"] = 66;
    params["blue_a_min"] = 130;
    params["blue_a_max"] = 145;
    params["blue_b_min"] = 95;
    params["blue_b_max"] = 105;

    params["orange_L_min"] = 166;
    params["orange_L_max"] = 191;
    params["orange_a_min"] = 135;
    params["orange_a_max"] = 142;
    params["orange_b_min"] = 160;
    params["orange_b_max"] = 174;

    params["black_L_min"] = 0;
    params["black_L_max"] = 21;
    params["black_a_min"] = 127;
    params["black_a_max"] = 133;
    params["black_b_min"] = 126;
    params["black_b_max"] = 134;

    params["red_L_min"] = 71;
    params["red_L_max"] = 97;
    params["red_a_min"] = 143;
    params["red_a_max"] = 153;
    params["red_b_min"] = 33;
    params["red_b_max"] = 154;

    params["purple_L_min"] = 171;
    params["purple_L_max"] = 197;
    params["purple_a_min"] = 131;
    params["purple_a_max"] = 141;
    params["purple_b_min"] = 108;
    params["purple_b_max"] = 123;

    params["saturation_threshold"] = 150;

    // Initialize combined result
    MIL_ID detection_result = M_NULL;

    // Measure execution time
    measure_execution_time([&]() {
        high_sat_detect(MilImage, detection_result, params);
    });
    MbufSave(SAVE_PATH, detection_result);
    // Display result

    std::cout << "所有颜色检测已完成并合并。按 <Enter> 退出。" << std::endl;
    getchar();

    // Free resources
    MbufFree(detection_result);
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    return 0;
}
