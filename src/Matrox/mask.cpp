
#include "mask.h"
// 读取二值化的单通道一位图片并生成掩膜
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

#include "utils.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;

// Function to convert an image into a mask based on sizeThreshold
vector<vector<uint8_t>> generateMask(
    const MIL_ID& inputImg,
    int outputWidth,
    int outputHeight,
    int sizeThreshold
) {
    cv::Mat image = mil2mat(inputImg);

    // Ensure the image is binary
    cv::threshold(image, image, 128, 255, cv::THRESH_BINARY);

    int imageWidth = image.cols;
    int imageHeight = image.rows;

    int blockWidth = imageWidth / outputWidth;
    int blockHeight = imageHeight / outputHeight;

    vector<vector<uint8_t>> mask(outputHeight, vector<uint8_t>(outputWidth, 0));

    for (int i = 0; i < outputHeight; ++i) {
        for (int j = 0; j < outputWidth; ++j) {
            int x_start = j * blockWidth;
            int y_start = i * blockHeight;
            int x_end = (j == outputWidth - 1) ? imageWidth : (j + 1) * blockWidth;
            int y_end = (i == outputHeight - 1) ? imageHeight : (i + 1) * blockHeight;

            cv::Mat block = image(cv::Rect(x_start, y_start, x_end - x_start, y_end - y_start));

            int whitePixelCount = cv::countNonZero(block);

            if (whitePixelCount > sizeThreshold) {
                mask[i][j] = 1;
            }
        }
    }

    return mask;
}
pair<vector<vector<uint8_t>>, vector<vector<uint8_t>>> applyRowRangeDelay(
    const vector<vector<uint8_t>>& mask,
    const vector<vector<uint8_t>>& tail,
    int rowRange
) {
    int outputHeight = (int)mask.size();
    int outputWidth = (int)mask[0].size();

    vector<vector<uint8_t>> mask_after_row_range(outputHeight, vector<uint8_t>(outputWidth, 0));
    vector<vector<uint8_t>> newTail(rowRange, vector<uint8_t>(outputWidth, 0));

    // 先将旧的 tail 映射到 mask_after_row_range 的顶部几行
    for (int i = 0; i < (int)tail.size(); ++i) {
        for (int j = 0; j < outputWidth; ++j) {
            if (i < outputHeight) {
                mask_after_row_range[i][j] = max(mask_after_row_range[i][j], tail[i][j]);
            }
        }
    }

    // 对当前 mask 应用 rowRange 的拖影效果
    for (int j = 0; j < outputWidth; ++j) {
        for (int i = 0; i < outputHeight; ++i) {
            if (mask[i][j] == 1) {
                // 从当前行 i 开始，向下扩展 rowRange 行
                int end_line = i + rowRange - 1;

                // 先处理仍在 mask 范围内的部分
                int inside_mask_end = min(end_line, outputHeight - 1);
                for (int line = i; line <= inside_mask_end; ++line) {
                    mask_after_row_range[line][j] = 1;
                }

                // 超出 mask 范围的行进入 tail
                if (end_line >= outputHeight) {
                    // 从 outputHeight 行开始的部分属于 tail
                    for (int line = outputHeight; line <= end_line; ++line) {
                        int tail_line_idx = line - outputHeight;
                        if (tail_line_idx >= 0 && tail_line_idx < (int)newTail.size()) {
                            newTail[tail_line_idx][j] = 1;
                        }
                    }
                }
            }
        }
    }

    return {mask_after_row_range, newTail};
}


// Updated wrapper function
pair<vector<vector<uint8_t>>, vector<vector<uint8_t>>> generateMaskWithTail(
    const MIL_ID& inputImg,
    const vector<vector<uint8_t>>& tail,
    int outputWidth,
    int outputHeight,
    int sizeThreshold = 10,
    int rowRange = 50
) {
    // Generate the mask from the image
    vector<vector<uint8_t>> mask = generateMask(inputImg, outputWidth, outputHeight, sizeThreshold);

    // Apply rowRange delay
    return applyRowRangeDelay(mask, tail, rowRange);
}

