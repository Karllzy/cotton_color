#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

// 读取二值化的单通道一位图片并生成掩膜
std::vector<std::vector<bool>> generateMaskFromImage(const std::string& imagePath, int widthBlocks, int heightBlocks, int threshold = 10) {
    // 读取图像
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);

    // 检查图像是否成功读取
    if (image.empty()) {
        std::cerr << "无法加载图像，请检查路径是否正确: " << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }

    // 确保图像是二值化的
    cv::threshold(image, image, 128, 255, cv::THRESH_BINARY);

    // 获取图像的宽度和高度
    int imageWidth = image.cols;
    int imageHeight = image.rows;

    // 计算每个块的宽度和高度
    int blockWidth = imageWidth / widthBlocks;
    int blockHeight = imageHeight / heightBlocks;

    // 创建掩膜矩阵
    std::vector<std::vector<bool>> mask(heightBlocks, std::vector<bool>(widthBlocks, false));

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

            // 如果白色像素数大于阈值，将该块标记为 true
            if (whitePixelCount > threshold) {
                mask[i][j] = true;
            }
        }
    }

    return mask;
}

int main() {
    // 指定图像路径
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\diguandai.png";

    // 设置分块数量和白色像素点阈值
    int widthBlocks = 24;
    int heightBlocks = 24;
    int threshold = 20;

    // 生成掩膜
    std::vector<std::vector<bool>> mask = generateMaskFromImage(imagePath, widthBlocks, heightBlocks, threshold);

    // 打印掩膜结果
    for (int i = 0; i < heightBlocks; ++i) {
        for (int j = 0; j < widthBlocks; ++j) {
            std::cout << (mask[i][j] ? "1 " : "0 ");
        }
        std::cout << std::endl;
    }

    return 0;
}
