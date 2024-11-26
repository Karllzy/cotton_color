//
// Created by zjc on 24-11-26.
//
#include <Matrox/utils.h>

#include "vector"
#include"iostream"
#include"string"
#include"Matrox/mask.h"
int main() {
    // 指定图像路径
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\suspect_mask.png";
    std::unordered_map<std::string, int> config = loadConfig("C:\\Users\\zjc\\Desktop\\config\\mask_config.txt");
    // 设置分块数量和白色像素点阈值
    int widthBlocks = config["widthBlocks"];
    int heightBlocks = config["heightBlocks"];
    int threshold = config["threshold"];

    // 输出参数值
    std::cout << "widthBlocks: " << widthBlocks << std::endl;

    // int widthBlocks = 24;
    // int heightBlocks = 24;
    // int threshold = 20;

    // 生成掩膜
    std::vector<std::vector<bool>> mask = generateMaskFromImage(imagePath, widthBlocks, heightBlocks, threshold);

    // 打印掩膜结果
    for (int i = 0; i < heightBlocks; ++i) {
        for (int j = 0; j < widthBlocks; ++j)
        {
            std::cout << (mask[i][j] ? "1 " : "0 ");
        }
        std::cout << std::endl;
    }

    return 0;
}