#include <iostream>
#include <Matrox/utils.h>
#include <opencv2/opencv.hpp>
#include "Mil.h"  // 引入 MIL 库

#define IMAGE_PATH MIL_TEXT(".\\test_imgs\\test_gray.png")
#define IMAGE_PATH_COLOR MIL_TEXT(".\\test_imgs\\8.bmp")
#define SAVE_PATH MIL_TEXT(".\\runs\\test_MILtoMat.png")

using namespace std;

void show_img_info(const MIL_ID mil_img) {
    // 获取图像尺寸和通道数
    MIL_INT width, height, channels;
    MbufInquire(mil_img, M_SIZE_X, &width);  // 获取图像宽度
    MbufInquire(mil_img, M_SIZE_Y, &height); // 获取图像高度
    MbufInquire(mil_img, M_SIZE_BAND, &channels); // 获取图像通道数
    cout << "Gray MIL Image loaded successfully!" << endl;
    cout << "Width: " << width << endl;
    cout << "Height: " << height << endl;
    cout << "Channels: " << channels << endl;
}

int main() {
    MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;
    MIL_ID MilImage = M_NULL;
    // 初始化 MIL 应用程序、系统和显示
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

    // 测试用例1：灰度图测试
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
    show_img_info(MilImage);
    cv::Mat cvImg = mil2mat(MilImage);
    cout << "MIL Image converted successfully!" << endl;
    cv::imshow("MIL Image", cvImg);   // 使用 OpenCV 显示图像
    // 等待按键并关闭窗口
    cv::waitKey(0);
    // 保存图像
    string savepath = ".\\runs\\test_MILtoMat_gray.png";
    if (!cv::imwrite(savepath, cvImg)) {
        cerr << "Failed to save image!" << endl;
    }

    //测试用例2：彩色图测试
    MIL_ID MilImage2 = M_NULL;
    MbufRestore(IMAGE_PATH_COLOR, MilSystem, &MilImage2);
    // show_img_info(MilImage2);
    cv::Mat cvImgColor = mil2mat(MilImage2);
    cout << "MIL Image converted successfully!" << endl;
    cv::imshow("MIL Image COLOR", cvImgColor);   // 使用 OpenCV 显示图像
    // 等待按键并关闭窗口
    cv::waitKey(0);
    // 保存图像
    savepath = ".\\runs\\test_MILtoMat_color.png";
    if (!imwrite(savepath, cvImgColor)) {
        cerr << "Failed to save image!" << endl;
    }

    MbufFree(MilImage); // 释放 MIL 图像资源
    MbufFree(MilImage2);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);  // 释放 MIL 系统资源
    return 0;
}
