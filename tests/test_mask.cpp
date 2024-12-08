//
// Created by zjc on 24-11-26.
//

#include "vector"
#include "iostream"
#include "string"
#include "Matrox/mask.h"
#include "CVDL/OnnxRunner.h"
#include "mil.h"

#define Image_PATH0 MIL_TEXT("test_imgs\\test_mask_imgs\\mask_1.png")
#define Image_PATH1 MIL_TEXT("test_imgs\\test_mask_imgs\\mask_6.png")
MIL_ID MilApplication, MilSystem, MilDisplay;

int main() {
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

    int widthBlocks = 24;
    int heightBlocks = 96;
    int threshold = 20;
    int rowRange = 1;

    std::vector<std::vector<uint8_t>> tail(0);

    std::cout << "=============================row 632, 1020 = 1 ===========================================" << std::endl;
    for (int imgIndex = 0; imgIndex < 2; imgIndex++) {
        MIL_ID MilImage = M_NULL;
        if (imgIndex == 0)
            MbufRestore(Image_PATH0, MilSystem, &MilImage);
        else
            MbufRestore(Image_PATH1, MilSystem, &MilImage);

        Timer timer1;
        timer1.restart();
        auto [mask, newTail] = generateMaskWithTail(MilImage, tail, widthBlocks, heightBlocks, threshold, rowRange);
        tail = newTail;
        timer1.printElapsedTime("Convert to mask spent");

        // 打印mask结果
        for (int r = 0; r < heightBlocks; ++r) {
            std::cout << "mask Row: " << r << " ";
            for (int c = 0; c < widthBlocks; ++c) {
                std::cout << (int)mask[r][c] << " ";
            }
            std::cout << std::endl;
        }
        if(imgIndex==0)
            {


        // 打印tail结果（使用tail.size()进行循环，避免越界）
        for (size_t r = 0; r < tail.size(); ++r) {
            std::cout << "tail 0 Row: " << r << " ";
            for (size_t c = 0; c < tail[r].size(); ++c) {
                std::cout << (int)tail[r][c] << " ";
            }
            std::cout << std::endl;
        }
           }
        MbufFree(MilImage);
    }

    std::cout << "=============================row 1020 = 1 ===========================================" << std::endl;
    rowRange = 10;
    tail = std::vector<std::vector<uint8_t>>(rowRange, std::vector<uint8_t>(widthBlocks, 0));
    for (int imgIndex = 0; imgIndex < 2; imgIndex++) {
        MIL_ID MilImage = M_NULL;
        if (imgIndex == 0)
            MbufRestore(Image_PATH0, MilSystem, &MilImage);
        else
            MbufRestore(Image_PATH1, MilSystem, &MilImage);

        Timer timer1;
        timer1.restart();
        auto [mask, newTail] = generateMaskWithTail(MilImage, tail, widthBlocks, heightBlocks, threshold, rowRange);
        tail = newTail;
        timer1.printElapsedTime("Convert to mask spent");

        // 打印mask结果
        for (int r = 0; r < heightBlocks; ++r) {
            std::cout << "Row: " << r << " ";
            for (int c = 0; c < widthBlocks; ++c) {
                std::cout << (int)mask[r][c] << " ";
            }
            std::cout << std::endl;
        }

        if(imgIndex==0)
        {
            // 打印tail结果（使用tail.size()进行循环，避免越界）
            for (size_t r = 0; r < tail.size(); ++r) {
                std::cout << "tail 0 Row: " << r << " ";
                for (size_t c = 0; c < tail[r].size(); ++c) {
                    std::cout << (int)tail[r][c] << " ";
                }
                std::cout << std::endl;
            }
        }

        MbufFree(MilImage);
    }

    std::cout << "=============================row 1030 = 1 ===========================================" << std::endl;
    rowRange = 20;
    tail = std::vector<std::vector<uint8_t>>(rowRange, std::vector<uint8_t>(widthBlocks, 0));
    for (int imgIndex = 0; imgIndex < 2; imgIndex++) {
        MIL_ID MilImage = M_NULL;
        if (imgIndex == 0)
            MbufRestore(Image_PATH0, MilSystem, &MilImage);
        else
            MbufRestore(Image_PATH1, MilSystem, &MilImage);

        Timer timer1;
        timer1.restart();
        auto [mask, newTail] = generateMaskWithTail(MilImage, tail, widthBlocks, heightBlocks, threshold, rowRange);
        tail = newTail;
        timer1.printElapsedTime("Convert to mask spent");

        // 打印mask结果
        for (int r = 0; r < heightBlocks; ++r) {
            std::cout << "Row: " << r << " ";
            for (int c = 0; c < widthBlocks; ++c) {
                std::cout << (int)mask[r][c] << " ";
            }
            std::cout << std::endl;
        }

        // 打印tail结果
        if(imgIndex==0)
        {
            // 打印tail结果（使用tail.size()进行循环，避免越界）
            for (size_t r = 0; r < tail.size(); ++r) {
                std::cout << "tail 0 Row: " << r << " ";
                for (size_t c = 0; c < tail[r].size(); ++c) {
                    std::cout << (int)tail[r][c] << " ";
                }
                std::cout << std::endl;
            }
        }

        MbufFree(MilImage);
    }

    std::cout << "=============================row 1030 = 1 ===========================================" << std::endl;
    rowRange = 40;
    tail = std::vector<std::vector<uint8_t>>(rowRange, std::vector<uint8_t>(widthBlocks, 0));
    for (int imgIndex = 0; imgIndex < 2; imgIndex++) {
        MIL_ID MilImage = M_NULL;
        if (imgIndex == 0)
            MbufRestore(Image_PATH0, MilSystem, &MilImage);
        else
            MbufRestore(Image_PATH1, MilSystem, &MilImage);

        Timer timer1;
        timer1.restart();
        auto [mask, newTail] = generateMaskWithTail(MilImage, tail, widthBlocks, heightBlocks, threshold, rowRange);
        tail = newTail;
        timer1.printElapsedTime("Convert to mask spent");

        // 打印mask结果
        for (int r = 0; r < heightBlocks; ++r) {
            std::cout << "Row: " << r << " ";
            for (int c = 0; c < widthBlocks; ++c) {
                std::cout << (int)mask[r][c] << " ";
            }
            std::cout << std::endl;
        }

        // 打印tail结果
        if(imgIndex==0)
        {
            // 打印tail结果（使用tail.size()进行循环，避免越界）
            for (size_t r = 0; r < tail.size(); ++r) {
                std::cout << "tail 0 Row: " << r << " ";
                for (size_t c = 0; c < tail[r].size(); ++c) {
                    std::cout << (int)tail[r][c] << " ";
                }
                std::cout << std::endl;
            }
        }

        MbufFree(MilImage);
    }

    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
    return 0;
}
