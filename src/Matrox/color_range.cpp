#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "utils.h"


// Optimized LabProcess function
void lab_process_raw(const MIL_ID& inputImage, MIL_ID& outputImageLab, const std::map<std::string, int>& params,
    const std::vector<std::string>& color_vector)
{
    MIL_ID MilLabImage = M_NULL, MilLChannel = M_NULL, MilAChannel = M_NULL, MilBChannel = M_NULL;
    MIL_ID lab_result=M_NULL;

    int denoising = params.at("lab_denoising");


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
    MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &lab_result);


    // Iterate over colors
    // 遍历颜色
    for (const auto& color : color_vector) {
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
        std::vector<int> lab_min_ps = {L_min, a_min, b_min};
        std::vector<int> lab_max_ps = {L_max, a_max, b_max};

        std::vector<int> lab_min_cv = psLabToOpenCVLab(lab_min_ps);
        std::vector<int> lab_max_cv = psLabToOpenCVLab(lab_max_ps);

        L_min = lab_min_cv[0];
        L_max = lab_max_cv[0];
        a_min = lab_min_cv[1];
        a_max = lab_max_cv[1];
        b_min = lab_min_cv[2];
        b_max = lab_max_cv[2];

        // 对每个通道进行二值化
        MimBinarize(MilLChannel, MilBinaryL, M_IN_RANGE, L_min, L_max);
        MimBinarize(MilAChannel, MilBinaryA, M_IN_RANGE, a_min, a_max);
        MimBinarize(MilBChannel, MilBinaryB, M_IN_RANGE, b_min, b_max);

        // 合并阈值结果
        MimArith(MilBinaryL, MilBinaryA, MilResultLab, M_AND);
        MimArith(MilResultLab, MilBinaryB, MilResultLab, M_AND);

        // 与输出图像合并
        MimArith(lab_result, MilResultLab, lab_result, M_OR);

    }
    MimClose(lab_result, MilResultLab, denoising, M_BINARY);
    MimOpen(MilResultLab, outputImageLab, denoising, M_BINARY);

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
    MbufFree(lab_result);
}


void lab_process(const MIL_ID& inputImage, MIL_ID& outputImageLab, const std::map<std::string, int>& params) {
    const std::vector<std::string> colors = {"green", "blue", "orange", "black", "red", "purple"};
    lab_process_raw(inputImage, outputImageLab, params, colors);
}


void hsv_process(const MIL_ID& inputImage, MIL_ID& outputImageHSV, const std::map<std::string, int>& params)
{
    MIL_ID MilHSVImage = M_NULL, MilHChannel = M_NULL, MilSChannel = M_NULL, MilVChannel = M_NULL;
    MIL_ID hsv_result = M_NULL;
    MIL_ID hsv_denoising = M_NULL;
    int saturationThreshold = params.at("saturation_threshold");
    int denoising = params.at("saturation_denoising");

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
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 1 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &hsv_result);
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
    MbufInquire(inputImage, M_SIZE_Y, M_NULL), 1 + M_UNSIGNED,
    M_IMAGE + M_PROC + M_DISP, &hsv_denoising);
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
    MbufInquire(inputImage, M_SIZE_Y, M_NULL), 1 + M_UNSIGNED,
    M_IMAGE + M_PROC + M_DISP, &outputImageHSV);

    // 对 S 通道进行阈值分割
    MimBinarize(MilSChannel, hsv_result, M_GREATER,
        saturationThreshold, M_NULL);

    MimClose(hsv_result, hsv_denoising, denoising, M_BINARY);
    MimOpen(hsv_denoising, outputImageHSV, denoising, M_BINARY);

    // 释放资源
    MbufFree(MilHChannel);
    MbufFree(MilSChannel);
    MbufFree(MilVChannel);
    MbufFree(MilHSVImage);
    MbufFree(hsv_result);
    MbufFree(hsv_denoising);
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


