//
// Created by zjc on 24-11-26.
//


#ifndef MASK_H
#define MASK_H

#include <opencv2/opencv.hpp>
#include <vector>
#include"mil.h"
#include <iostream>


std::vector<std::vector<uint8_t>> generateMaskFromImage(const MIL_ID& inputImage, int widthBlocks, int heightBlocks, int threshold , int rowRange) ;
#endif //MASK_H
