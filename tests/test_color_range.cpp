#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "Matrox/utils.h"
#include "Matrox/color_range.h"

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton_image\\174.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\diguandai.png")

// Global variables
MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;

int main()
{
    // Initialize MIL application
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
        M_NULL);

    // Load input image
    MIL_ID MilImage = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);

    // Define color ranges
    std::map<std::string, int> params;
    params["green_L_min"] = 27;
    params["green_L_max"] = 49;
    params["green_a_min"] = -27;
    params["green_a_max"] = -8;
    params["green_b_min"] = 2;
    params["green_b_max"] = 12;

    params["blue_L_min"] = 18;
    params["blue_L_max"] = 26;
    params["blue_a_min"] = 2;
    params["blue_a_max"] = 17;
    params["blue_b_min"] = -33;
    params["blue_b_max"] = -23;

    params["orange_L_min"] = 65;
    params["orange_L_max"] = 75;
    params["orange_a_min"] = 7;
    params["orange_a_max"] = 14;
    params["orange_b_min"] = 32;
    params["orange_b_max"] = 46;

    params["black_L_min"] = 0;
    params["black_L_max"] = 8;
    params["black_a_min"] = -1;
    params["black_a_max"] = 5;
    params["black_b_min"] = -2;
    params["black_b_max"] = 6;

    params["red_L_min"] = 28;
    params["red_L_max"] = 38;
    params["red_a_min"] = 15;
    params["red_a_max"] = 25;
    params["red_b_min"] = -95;
    params["red_b_max"] = 26;

    params["purple_L_min"] = 67;
    params["purple_L_max"] = 77;
    params["purple_a_min"] = 3;
    params["purple_a_max"] = 13;
    params["purple_b_min"] = -20;
    params["purple_b_max"] = -5;
    params["lab_denoising"] = 1;

    params["saturation_threshold"] = 150;
    params["saturation_denoising"] = 1;

    // Initialize combined result
    MIL_ID detection_result = M_NULL;

    // Measure execution time
    measure_execution_time([&]() {
        high_sat_detect(MilImage, detection_result, params);
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