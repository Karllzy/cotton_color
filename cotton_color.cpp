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
std::wstring openFileDialog() {
    // 初始化文件选择对话框
    OPENFILENAMEW ofn;       // 使用宽字符版本的结构
    wchar_t szFile[260] = {0};    // 存储选择的文件路径

    // 设置 OPENFILENAMEW 结构的默认值
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;  // 设置文件路径缓冲区
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"Image Files\0*.BMP;*.JPG;*.JPEG;*.PNG;*.GIF\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;  // 不需要单独的文件名
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;  // 使用默认初始目录
    ofn.lpstrTitle = L"Select an image file";  // 对话框标题
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // 打开文件选择对话框
    if (GetOpenFileNameW(&ofn) == TRUE) {
        return szFile;  // 返回选中的文件路径
    }

    return L"";  // 如果用户取消，返回空字符串
}

/**
 * @brief 读取图像文件，支持 Unicode 路径。
 *
 * @return 加载的图像，类型为 cv::Mat。如果加载失败，返回空的 Mat。
 */
Mat readImage() {
    // 读取输入图像路径
    std::wstring imagePath = openFileDialog();

    if (imagePath.empty()) {
        wcout << L"No file selected or user cancelled." << endl;
        return Mat();
    }

    // 使用 Windows API 打开文件
    HANDLE hFile = CreateFileW(imagePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wcout << L"Error: Could not open file." << endl;
        return Mat();
    }

    // 获取文件大小
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        wcout << L"Error: Could not get file size." << endl;
        CloseHandle(hFile);
        return Mat();
    }

    if (fileSize.QuadPart > MAXDWORD) {
        wcout << L"Error: File size too large." << endl;
        CloseHandle(hFile);
        return Mat();
    }

    DWORD dwFileSize = static_cast<DWORD>(fileSize.QuadPart);

    // 读取文件内容到缓冲区
    std::vector<BYTE> buffer(dwFileSize);
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer.data(), dwFileSize, &bytesRead, NULL) || bytesRead != dwFileSize) {
        wcout << L"Error: Could not read file." << endl;
        CloseHandle(hFile);
        return Mat();
    }

    CloseHandle(hFile);

    // 使用 OpenCV 从内存缓冲区读取图像
    Mat image = imdecode(buffer, IMREAD_COLOR);

    if (image.empty()) {
        wcout << L"Error: Could not decode image." << endl;
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
    double scaleFactor = 0.6;  // 将图像放大1.5倍

    // 显示原图和检测到的绿色区域，使用缩放因子
    showImage("Original Image", inputImage, scaleFactor);
    showImage("Detected Vibrant Green", outputImage, scaleFactor);

    // 等待用户按键
    waitKey(0);
    return 0;
}
