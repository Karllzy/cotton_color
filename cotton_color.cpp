#include <opencv2/opencv.hpp>
#include <iostream>
#include <map>
#include <string>
#include <windows.h>
#include <commdlg.h>  // 包含文件对话框相关的函数

using namespace cv;
using namespace std;

/**
 * @brief 鲜艳绿色检测函数，通过指定的 Lab 色彩范围检测输入图像中的绿色区域。
 *
 * @param inputImage 输入图像，类型为 cv::Mat，要求为 BGR 色彩空间。
 * @param outputImage 输出图像，类型为 cv::Mat，输出图像将包含检测到的绿色区域。
 * @param params 参数映射，用于传递各种可配置的参数，如绿色阈值等。
 */
void vibrantGreenDetection(const Mat& inputImage, Mat& outputImage, const map<string, int>& params) {
    // 从参数映射中获取绿色阈值
    int green = params.at("green");

    // 将输入图像从 BGR 转换为 Lab
    Mat lab_image;
    cvtColor(inputImage, lab_image, cv::COLOR_BGR2Lab);

    // 定义偏绿色的 Lab 范围（具体值可能需要调整）
    Scalar lower_green_lab(101, 101, 95);
    Scalar upper_green_lab(135, 120, green);

    // 创建掩膜
    Mat mask_lab;
    inRange(lab_image, lower_green_lab, upper_green_lab, mask_lab);

    // 通过掩膜提取偏绿色部分，将结果存储在 outputImage 中
    bitwise_and(inputImage, inputImage, outputImage, mask_lab);
}
void vibrantColorDetection(const Mat& inputImage, Mat& outputImage, const map<string, int>& params) {
    // 从参数映射中获取饱和度阈值
    int saturationThreshold = params.at("saturationThreshold");

    // 将输入图像从 BGR 转换为 HSV
    Mat hsvImage;
    cvtColor(inputImage, hsvImage, COLOR_BGR2HSV);

    // 分离 HSV 图像的各个通道
    Mat channels[3];
    split(hsvImage, channels);

    // 获取饱和度通道 (S)
    Mat saturation = channels[1];

    // 创建输出图像，将饱和度大于阈值的区域标记为杂质
    outputImage = Mat::zeros(inputImage.size(), CV_8UC1);

    // 对饱和度图像应用阈值处理
    threshold(saturation, outputImage, saturationThreshold, 255, THRESH_BINARY);
}
string openFileDialog() {
    // 初始化文件选择对话框
    OPENFILENAME ofn;       // 文件对话框结构
    wchar_t szFile[260];    // 存储选择的文件路径

    // 设置 OPENFILENAME 结构的默认值
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"Image Files\0*.BMP;*.JPG;*.JPEG;*.PNG;*.GIF\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"Select an image file";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // 打开文件选择对话框
    if (GetOpenFileName(&ofn) == TRUE) {
        // 将 wchar_t 转换为 string
        wstring ws(szFile);
        string filePath(ws.begin(), ws.end());
        return filePath;
    }

    return "";  // 如果用户取消，返回空字符串
}

Mat readImage() {
    // 读取输入图像
    string imagePath = openFileDialog();

    if (imagePath.empty()) {
        cout << "No file selected or user cancelled." << endl;
        return Mat();
    }

    // 使用 OpenCV 读取选中的图片
    Mat image = imread(imagePath);

    if (image.empty()) {
        cout << "Error: Could not load image." << endl;
        return Mat();
    }

    return image;
}

// 辅助函数，用于调整图像大小并显示，支持等比例放大
void showImage(const string& windowName, const Mat& img, double scaleFactor = 1.0) {
    Mat resizedImg;
    int newWidth = static_cast<int>(img.cols * scaleFactor);
    int newHeight = static_cast<int>(img.rows * scaleFactor);

    // 调整图像大小
    resize(img, resizedImg, Size(newWidth, newHeight));

    // 显示图像
    imshow(windowName, resizedImg);
}

int main() {
    // 读取输入图像
    Mat inputImage = readImage();

    if (inputImage.empty()) {
        cout << "Error: Could not load image." << endl;
        return -1;
    }

    // 创建输出图像
    Mat outputImage;

    // 使用 map 模拟 JSON 参数传递
    map<string, int> params;
    params["green"] = 134;  // 设置绿色阈值

    // 调用鲜艳绿色检测函数
    vibrantGreenDetection(inputImage, outputImage, params);

    // 定义缩放因子，1.0 表示原始大小，>1.0 表示放大，<1.0 表示缩小
    double scaleFactor = 1.5;  // 将图像放大1.5倍

    // 显示原图和检测到的绿色区域，使用缩放因子
    showImage("Original Image", inputImage, scaleFactor);
    showImage("Detected Vibrant Green", outputImage, scaleFactor);

    // 等待用户按键
    waitKey(0);
    return 0;
}
