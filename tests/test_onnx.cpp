#include "vector"
#include"iostream"
#include"string"
#include"Matrox/utils.h"
#include"opencv2/opencv.hpp"

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\8.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")

// int main() {
//     MIL_ID MilImage = M_NULL;
//     MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;
//
//     MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
//       M_NULL);
//     MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
//     cv::Mat opencvImage = milToMat(MilImage);
//     imshow("opencv", opencvImage);
//     // MosGetch();
//
//     return 0;
// }
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
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL,M_NULL);
        return -1;
    }

    // 转换并显示
    cv::Mat opencvImage = milToMat(MilImage);
    if (!opencvImage.empty()) {
        cv::imshow("opencv", opencvImage);
        cv::waitKey(0);
    }
    std:: string savepath="C:\\Users\\zjc\\Desktop\\suspect.png";
    cv::imwrite(savepath,opencvImage);
    // 释放资源
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL,M_NULL);  // 使用 MappFreeDefault 代替 MappFree

    return 0;
}

// int main() {
//     cv::Mat img = cv::imread("C:\\Users\\zjc\\Desktop\\suspect.png");
//     if (img.empty()) {
//         std::cout << "图像加载失败!" << std::endl;
//         return -1;
//     }
//
//     // 处理图像
//     processImage(img);
// }