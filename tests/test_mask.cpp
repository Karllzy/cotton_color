//
// Created by zjc on 24-11-26.
//
// #include <Matrox/utils.h>

#include "vector"
#include"iostream"
#include"string"
#include"Matrox/mask.h"

#include"mil.h"
#define Image_PATH3 MIL_TEXT("C:\\Users\\zjc\\Desktop\\mask_5.png")
MIL_ID MilApplication, MilSystem, MilDisplay;

int main() {
    // 指定图像路径
    // std::string imagePath = "C:\\Users\\zjc\\Desktop\\mask_1.png";
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);
    MIL_ID MilImage = M_NULL, MilHighSatResult = M_NULL, MilTemplateMatchingResult = M_NULL;
    MbufRestore(Image_PATH3, MilSystem, &MilImage);
    // std::unordered_map<std::string, int> config = loadConfig("C:\\Users\\zjc\\Desktop\\config\\mask_config.txt");
    // // 设置分块数量和白色像素点阈值
    // int widthBlocks = config["widthBlocks"];
    // int heightBlocks = config["heightBlocks"];
    // int threshold = config["threshold"];

    // 输出参数值
    // std::cout << "widthBlocks: " << widthBlocks << std::endl;

    int widthBlocks = 24;
    int heightBlocks = 1024;
    int threshold = 20;
    int rowRange = 50;     // 后续50行设置为

    // 生成掩膜
    std::vector<std::vector<uint8_t>> mask = generateMaskFromImage(MilImage, widthBlocks, heightBlocks, threshold, rowRange);

    // 打印掩膜结果
    for (int i = 0; i < heightBlocks; ++i) {
        for (int j = 0; j < widthBlocks; ++j) {
            std::cout << (int)mask[i][j] << " ";
        }
        std::cout << std::endl;
    }
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    return 0;
}