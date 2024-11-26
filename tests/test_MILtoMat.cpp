#include <iostream>
#include <opencv2/opencv.hpp>
#include "Mil.h"  // 引入 MIL 库

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\8.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")

int main() {
    MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;
    MIL_ID MilImage = M_NULL;

    // 初始化 MIL 应用程序、系统和显示
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);
    if (MilApplication == M_NULL || MilSystem == M_NULL || MilDisplay == M_NULL) {
        std::cerr << "MIL Initialization failed!" << std::endl;
        return -1;
    }

    // 加载图像
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
    if (MilImage == M_NULL) {
        std::cerr << "Failed to load MIL image!" << std::endl;
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }

    // 获取图像尺寸和通道数
    int width, height, channels;
    MbufInquire(MilImage, M_SIZE_X, &width);  // 获取图像宽度
    MbufInquire(MilImage, M_SIZE_Y, &height); // 获取图像高度
    MbufInquire(MilImage, M_SIZE_BAND, &channels); // 获取图像通道数

    // 为图像数据分配缓冲区
    unsigned char* m_AvsBuffer = (unsigned char*)malloc(width * height * channels);

    if (m_AvsBuffer == NULL) {
        std::cerr << "Memory allocation for image buffer failed!" << std::endl;
        MbufFree(MilImage);
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }

    // 获取 MIL 图像数据
    MbufGet(MilImage, m_AvsBuffer);

    // 将 MIL 图像数据转换为 OpenCV 格式
    cv::Mat cvImage;

    if (channels == 1) {
        // 灰度图像（1 通道）
        cvImage = cv::Mat(height, width, CV_8UC1, m_AvsBuffer);
    } else if (channels == 3) {
        // 彩色图像（3 通道，BGR）
        cvImage = cv::Mat(height, width, CV_8UC3, m_AvsBuffer);
    } else {
        std::cerr << "Unsupported number of channels: " << channels << std::endl;
        free(m_AvsBuffer);
        MbufFree(MilImage);
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }

    // 显示图像
    MdispSelect(MilDisplay, MilImage);  // 在 MIL 显示中显示图像
    cv::imshow("MIL Image", cvImage);   // 使用 OpenCV 显示图像

    // 等待按键并关闭窗口
    cv::waitKey(0);

    // 保存图像
    std::string savepath = "C:\\Users\\zjc\\Desktop\\suspect.png";
    if (!cv::imwrite(savepath, cvImage)) {
        std::cerr << "Failed to save image!" << std::endl;
    }

    // 释放资源
    free(m_AvsBuffer);  // 释放缓冲区
    MbufFree(MilImage); // 释放 MIL 图像资源
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);  // 释放 MIL 系统资源

    return 0;
}
