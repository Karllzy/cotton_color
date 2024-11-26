//
// Created by zjc on 24-11-20.
//
#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "Matrox/utils.h"
#include "Matrox/template_matching.h"

#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton_image_new\\357.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")


// Global variables
MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;


int main() {
    using namespace std;

    std::map<std::string, int> params;
    read_params_from_file("C:\\Users\\zjc\\Desktop\\config\\template_color_config.txt", params);
    // Initialize MIL application
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
        M_NULL);

    // Load input image
    MIL_ID MilImage = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);

    // Initialize combined result
    MIL_ID detection_result = M_NULL;
    MIL_ID detection_resize = M_NULL;



    MIL_ID output_Image= M_NULL;
    TemplateMatcher matcher(MilSystem, MilDisplay, params);

    // Measure execution time
    measure_execution_time([&]()
        {
        pre_process(MilImage, detection_result, params);
        MbufAlloc2d(MilSystem, MbufInquire(detection_result, M_SIZE_X, M_NULL)/2,
           MbufInquire(detection_result, M_SIZE_Y, M_NULL)/2, 1 + M_UNSIGNED,
           M_IMAGE + M_PROC, &detection_resize);
        MimResize(detection_result,detection_resize,0.5,0.5,M_DEFAULT);

        matcher.LoadTemplate(params);
        matcher.FindTemplates(detection_resize,output_Image,params);

    });
    MbufSave(SAVE_PATH, detection_result);
    // Display result

    std::cout << "所有颜色检测已完成并合并。按 <Enter> 退出。" << std::endl;
    getchar();

    MbufFree(detection_result);
    MbufFree(MilImage);
    return 0;
}