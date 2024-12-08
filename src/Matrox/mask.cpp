
#include "mask.h"
// 读取二值化的单通道一位图片并生成掩膜
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

#include "utils.h"

std::vector<std::vector<uint8_t>> generateMaskFromImage(const MIL_ID& inputImage, int widthBlocks, int heightBlocks, int threshold = 10, int rowRange = 50) {
    // 读取图像
    cv::Mat image = mil2mat(inputImage);
    //
    // // 检查图像是否成功读取
    // if (image.empty()) {
    //     std::cerr << "无法加载图像，请检查路径是否正确: " << imagePath << std::endl;
    //     exit(EXIT_FAILURE);
    // }

    // 确保图像是二值化的
    cv::threshold(image, image, 128, 255, cv::THRESH_BINARY);

    // 获取图像的宽度和高度
    int imageWidth = image.cols;
    int imageHeight = image.rows;

    // 计算每个块的宽度和高度
    int blockWidth = imageWidth / widthBlocks;
    int blockHeight = imageHeight / heightBlocks;

    // 创建掩膜矩阵 (uint8_t 类型)
    std::vector<std::vector<uint8_t>> mask(heightBlocks, std::vector<uint8_t>(widthBlocks, 0));

    // 遍历每个块并统计白色像素点的数量
    for (int i = 0; i < heightBlocks; ++i) {
        for (int j = 0; j < widthBlocks; ++j) {
            // 计算块的起始和结束位置
            int x_start = j * blockWidth;
            int y_start = i * blockHeight;
            int x_end = (j == widthBlocks - 1) ? imageWidth : (j + 1) * blockWidth;
            int y_end = (i == heightBlocks - 1) ? imageHeight : (i + 1) * blockHeight;

            // 提取当前块
            cv::Mat block = image(cv::Rect(x_start, y_start, x_end - x_start, y_end - y_start));

            // 统计块中白色像素的数量
            int whitePixelCount = cv::countNonZero(block);

            // 如果白色像素数大于阈值，将该块标记为 255
            if (whitePixelCount > threshold) {
                mask[i][j] = 1;
            }
        }
    }

    // 遍历每一列，处理规则：当某列出现第一个1时，将其后rowRange行全部置为255
    for (int j = 0; j < widthBlocks; ++j) {
        bool marked = false;  // 标记当前列是否已经处理过第一个1

        for (int i = 0; i < heightBlocks; ++i) {
            if (mask[i][j] == 1&& !marked) {
                // 找到第一个1，处理后面rowRange行
                for (int k = i; k < std::min(i + rowRange, heightBlocks); ++k) {
                    mask[k][j] = 1;
                }
                marked = true;  // 标记为已处理，后续连续的1不再处理
            }
        }
    }

    return mask;
}

