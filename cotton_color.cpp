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
 * 此函数将输入图像从 BGR 色彩空间转换到 HSV 色彩空间，并提取出饱和度 (S) 通道。然后，通过设置饱和度阈值，
 * 来检测图像中饱和度大于阈值的区域，标记为输出图像的鲜艳颜色区域。
 *
 * @param inputImage 输入图像，类型为 cv::Mat，要求为 BGR 色彩空间。
 * @param outputImage 输出图像，类型为 cv::Mat，输出图像将标记出鲜艳颜色区域，原始图像尺寸。
 * @param saturationThreshold 饱和度阈值，类型为 int，用于过滤低饱和度的区域，范围通常为 0 到 255。
 *                            饱和度高于此阈值的区域将被认为是鲜艳的颜色。
 *
 * @note 饱和度阈值越高，输出图像将只保留更为鲜艳的区域。
 *       若饱和度阈值过低，可能会检测到过多的区域。
 */
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


void blackColorDetection(const Mat& inputImage, Mat& outputImage, const map<string, int>& params)
{
    outputImage = Mat::zeros(inputImage.size(), CV_8UC1);
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


void test() {}
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
    params["saturationThreshold"] = 100;  // 设置饱和度阈值为100

    // 调用鲜艳颜色检测函数
    vibrantColorDetection(inputImage, outputImage, params);

    // 显示原图和检测到的鲜艳区域
    imshow("Original Image", inputImage);
    imshow("Detected Vibrant Colors", outputImage);

    // 等待用户按键
    waitKey(0);
    return 0;
}