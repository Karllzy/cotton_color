#include <opencv2/opencv.hpp>
#include <iostream>
#include <map>
#include <string>


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


int main() {
    // 读取输入图像
    Mat inputImage = imread("C:\\Program Files\\Matrox Imaging\\Images\\test.bmp");

    if (inputImage.empty()) {
        cout << "Error: Could not load image!" << endl;
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
