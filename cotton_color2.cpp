#include <opencv2/opencv.hpp>
#include <iostream>
#include <map>
#include <string>
#include <windows.h>
#include <commdlg.h>  // 包含文件对话框相关的函数

using namespace cv;
using namespace std;

/**
 * @brief 鲜艳色彩检测函数，通过饱和度阈值检测输入图像中鲜艳的颜色区域。
 *
 * @param inputImage 输入图像，类型为 cv::Mat，要求为 BGR 色彩空间。
 * @param outputImage 输出图像，类型为 cv::Mat，输出图像将标记出鲜艳颜色区域，原始图像尺寸。
 * @param params 参数映射，用于传递各种可配置的参数，如饱和度阈值等。
 */
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

/**
 * @brief 打开文件对话框，返回选中文件的路径。
 *
 * @return 选中文件的完整路径，类型为 std::wstring。如果用户取消选择，返回空字符串。
 */
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

int main() {
    // 读取输入图像
    Mat inputImage = readImage();

    if (inputImage.empty()) {
        cout << "Error: Could not load image." << endl;
        return -1;
    }

    // 创建输出图像
    Mat outputImage;

    // 使用 map 模拟参数传递
    map<string, int> params;
    params["saturationThreshold"] = 134;  // 设置饱和度阈值为 100

    // 调用鲜艳颜色检测函数
    vibrantColorDetection(inputImage, outputImage, params);

    // 显示原图和检测到的鲜艳区域
    imshow("Original Image", inputImage);
    imshow("Detected Vibrant Colors", outputImage);

    // 等待用户按键
    waitKey(0);
    return 0;
}
