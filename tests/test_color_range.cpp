#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "Matrox/utils.h"
#include "Matrox/color_range.h"

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\123.bmp")
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
    read_params_from_file("..\\config\\color_range_config.txt", params);
    // Initialize combined result
    MIL_ID detection_result = M_NULL;
    for(int i = 0; i < 50; i++) {
        // Measure execution time
        measure_execution_time([&]() {
            high_sat_detect(MilImage, detection_result, params);
        });
        MbufSave(convert_to_wstring("./runs/test_color_range" + std::to_string(i) + ".png"),
            detection_result);
    }
    // Display result

    std::cout << "所有颜色检测已完成并合并。按 <Enter> 退出。" << std::endl;
    getchar();

    // Free resources
    MbufFree(detection_result);
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    return 0;
}