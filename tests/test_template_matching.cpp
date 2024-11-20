//
// Created by zjc on 24-11-20.
//
#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "Matrox/utils.h"
#include "Matrox/template_matching.h"

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton2.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")

// Global variables
MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;


int main() {
    using namespace std;
    map<string, int> params;
    params["cotton_L_min"] = 40;
    params["cotton_L_max"] = 90;
    params["cotton_a_min"] = -5;
    params["cotton_a_max"] = 6;
    params["cotton_b_min"] = -5;
    params["cotton_b_max"] = 30;

    params["background_L_min"] = 95;
    params["background_L_max"] = 100;
    params["background_a_min"] = -3;
    params["background_a_max"] = 3;
    params["background_b_min"] = -3;
    params["background_b_max"] = 3;

    params["cotton_denoising"] = 1;

    // Initialize MIL application
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
        M_NULL);

    // Load input image
    MIL_ID MilImage = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);

    // Initialize combined result
    MIL_ID detection_result = M_NULL;

    // Measure execution time
    measure_execution_time([&]() {
        pre_process(MilImage, detection_result, params);
        template_matching(detection_result, detection_result, params);
    });
    MbufSave(SAVE_PATH, detection_result);
    // Display result

    std::cout << "所有颜色检测已完成并合并。按 <Enter> 退出。" << std::endl;
    getchar();

    // Free resources
    MbufFree(detection_result);
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
    return 0;
}