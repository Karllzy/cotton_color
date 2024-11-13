//
// Created by zjc on 24-11-12.
//

#include <mil.h>
#include <iostream>
#include <chrono>
#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton2.bmp")

// 全局变量，方便在各个函数中使用
MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;


// 时间测量模板函数
template <typename Func>
void measureExecutionTime(Func func) {
    // 获取当前时间作为起点
    auto start = std::chrono::high_resolution_clock::now();

    // 执行传入的函数
    func();

    // 获取当前时间作为结束点
    auto end = std::chrono::high_resolution_clock::now();

    // 计算时间差并转换为毫秒
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Function execution time: " << duration.count() << " milliseconds" << std::endl;
}

// LabProcess 函数，支持通过参数控制阈值范围，提供默认值
void LabProcess(MIL_ID& inputImage, MIL_ID& outputImageLab,
    MIL_DOUBLE lowerL = 101.0, MIL_DOUBLE upperL = 135.0,
    MIL_DOUBLE lowerA = 101.0, MIL_DOUBLE upperA = 120.0,
    MIL_DOUBLE lowerB = 95.0, MIL_DOUBLE upperB = 134.0)
{
    MIL_ID MilLabImage = M_NULL, MilLChannel = M_NULL, MilAChannel = M_NULL, MilBChannel = M_NULL;
    MIL_ID MilBinaryL = M_NULL, MilBinaryA = M_NULL, MilBinaryB = M_NULL;

    // 检查输入图像的通道数
    MIL_INT NumBands = 0;
    MbufInquire(inputImage, M_SIZE_BAND, &NumBands);
    if (NumBands != 3)
    {
        printf("输入图像不是 3 通道图像，请提供彩色图像。\n");
        return;
    }

    // 分配用于存储 Lab 图像的缓冲区
    MbufAllocColor(MbufInquire(inputImage, M_OWNER_SYSTEM, M_NULL), 3,
        MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL),
        8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP,
        &MilLabImage);

    // 将图像从 sRGB 转换到 Lab
    MimConvert(inputImage, MilLabImage, M_SRGB_TO_LAB);

    // 创建 Lab 通道的子缓冲区
    MbufChildColor(MilLabImage, 0, &MilLChannel);
    MbufChildColor(MilLabImage, 1, &MilAChannel);
    MbufChildColor(MilLabImage, 2, &MilBChannel);

    // 分配二值图像缓冲区
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &MilBinaryL);
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &MilBinaryA);
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &MilBinaryB);

    // 对每个通道进行阈值分割
    MimBinarize(MilLChannel, MilBinaryL, M_IN_RANGE, lowerL, upperL);
    MimBinarize(MilAChannel, MilBinaryA, M_IN_RANGE, lowerA, upperA);
    MimBinarize(MilBChannel, MilBinaryB, M_IN_RANGE, lowerB, upperB);

    // 分配输出图像缓冲区
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &outputImageLab);

    // 将结果合并
    MimArith(MilBinaryL, MilBinaryA, outputImageLab, M_AND);
    MimArith(outputImageLab, MilBinaryB, outputImageLab, M_AND);

    // 释放资源
    MbufFree(MilBinaryL);
    MbufFree(MilBinaryA);
    MbufFree(MilBinaryB);
    MbufFree(MilLChannel);
    MbufFree(MilAChannel);
    MbufFree(MilBChannel);
    MbufFree(MilLabImage);
}

// HSVProcess 函数，支持通过参数控制饱和度阈值，提供默认值
void HSVProcess(MIL_ID& inputImage, MIL_ID& outputImageHSV, MIL_DOUBLE saturationThreshold = 120.0)
{
    MIL_ID MilHSVImage = M_NULL, MilHChannel = M_NULL, MilSChannel = M_NULL, MilVChannel = M_NULL;

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
    MimBinarize(MilSChannel, outputImageHSV, M_GREATER, saturationThreshold, M_NULL);

    // 释放资源
    MbufFree(MilHChannel);
    MbufFree(MilSChannel);
    MbufFree(MilVChannel);
    MbufFree(MilHSVImage);
}

// 综合测试函数，调用 LabProcess 和 HSVProcess 并合并结果
void test_hsv(MIL_ID& inputImage,
    MIL_DOUBLE lowerL = 101.0, MIL_DOUBLE upperL = 135.0,
    MIL_DOUBLE lowerA = 101.0, MIL_DOUBLE upperA = 120.0,
    MIL_DOUBLE lowerB = 95.0, MIL_DOUBLE upperB = 134.0,
    MIL_DOUBLE saturationThreshold = 120.0)
{
    MIL_ID MilResultLab = M_NULL, MilResultHSV = M_NULL, MilCombinedResult = M_NULL;

    // 调用 LabProcess
    LabProcess(inputImage, MilResultLab, lowerL, upperL, lowerA, upperA, lowerB, upperB);

    // 调用 HSVProcess
    HSVProcess(inputImage, MilResultHSV, saturationThreshold);

    // 分配合并结果的缓冲区
    MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
        MbufInquire(inputImage, M_SIZE_Y, M_NULL), 8 + M_UNSIGNED,
        M_IMAGE + M_PROC + M_DISP, &MilCombinedResult);

    // 合并 Lab 和 HSV 的结果（取“或”运算）
    MimArith(MilResultLab, MilResultHSV, MilCombinedResult, M_OR);

    //// 显示合并后的结果图像
    MdispSelect(MilDisplay, MilCombinedResult);

    //// 等待用户查看处理后的图像
    printf("图像已处理并合并，按下 <Enter> 退出程序。\n");
    getchar();

    // 释放资源
    MbufFree(MilResultLab);
    MbufFree(MilResultHSV);
    MbufFree(MilCombinedResult);
}


int main()
{
    MIL_ID MilImage = M_NULL;

    // 初始化 MIL 应用程序
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

    // 加载输入图像
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);

    // 使用 lambda 表达式测量 test_hsv() 的执行时间
    measureExecutionTime([&]() {
        test_hsv(MilImage);
        });


    // 释放资源
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    return 0;
}