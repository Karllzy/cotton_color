//
// Created by zjc on 24-11-26.
//


#ifndef MASK_H
#define MASK_H

#include <opencv2/opencv.hpp>
#include <vector>
#include"mil.h"
#include <iostream>
using namespace std;


pair<vector<vector<uint8_t>>, vector<vector<uint8_t>>> generateMaskWithTail(
    const MIL_ID& inputImg,
    const vector<vector<uint8_t>>& tail,
    int outputWidth,
    int outputHeight,
    int sizeThreshold,
    int rowRange
);

#endif //MASK_H
