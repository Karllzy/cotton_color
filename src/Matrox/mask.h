//
// Created by zjc on 24-11-26.
//


#ifndef MASK_H
#define MASK_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

std::vector<std::vector<bool>> generateMaskFromImage(const std::string& imagePath, int widthBlocks, int heightBlocks, int threshold);


#endif //MASK_H
