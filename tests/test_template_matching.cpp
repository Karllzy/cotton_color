//
// Created by zjc on 24-11-20.
//
#include <iostream>
#include <map>
#include <mil.h>
#include <string>
#include "Matrox/utils.h"
#include "Matrox/template_matching.h"
#include <filesystem>
#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton_image_new\\357.bmp")
#define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")


// Global variables
MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;
namespace fs = std::filesystem;


void LoadImagesFromFolder(const std::string& folderPath)
{
    // 遍历文件夹中的所有文件
    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        // 只处理图片文件（例如 .jpg, .png, .bmp 等）
        if (entry.is_regular_file() &&
            (entry.path().extension() == ".jpg" ||
             entry.path().extension() == ".png" ||
             entry.path().extension() == ".bmp"))
        {
            MIL_ID MilImage = M_NULL; // 每次读取新图像时都创建新的 MilImage 对象
            std::string imagePath = entry.path().string();  // 获取文件的路径

            // 使用 MIL_TEXT 宏将 imagePath 转换为适合 MIL 函数的字符串格式

            std::map<std::string, int> params;
            read_params_from_file("C:\\Users\\zjc\\Desktop\\config\\template_color_config.txt", params);
            // 调用 MbufRestore 加载图像
            MbufRestore(convert_to_wstring(imagePath), MilSystem, &MilImage);

            if (MilImage != M_NULL)
            {
                std::cout << "Successfully loaded image: " << imagePath << std::endl;
                // Initialize combined result
                MIL_ID detection_result = M_NULL;
                MIL_ID detection_resize = M_NULL;
                cv::Mat template_result;
                MIL_ID output_Image= M_NULL;
                TemplateMatcher matcher(params);
                matcher.LoadConfig("C:\\Users\\zjc\\Desktop\\config\\template_config.txt");
                // Measure execution time
                for (int i = 0; i <12; i++) {
                    measure_execution_time([&]()
                        {
                        pre_process(MilImage, detection_result, params);
                        MbufAlloc2d(MilSystem, MbufInquire(detection_result, M_SIZE_X, M_NULL)/2,
                           MbufInquire(detection_result, M_SIZE_Y, M_NULL)/2, 8 + M_UNSIGNED,
                           M_IMAGE + M_PROC, &detection_resize);
                        MimResize(detection_result,detection_resize,0.5,0.5,M_DEFAULT);
                        matcher.FindTemplates(detection_resize,output_Image,params);
                        template_result = mil2mat(output_Image);
                        cv::imwrite("./runs/template_result222"+std::to_string(i)+".png", template_result);
                    });
                }
                MbufFree(detection_result);
                MbufFree(MilImage);
                MbufFree(output_Image);
            }
            else
            {
                std::cerr << "Failed to load image: " << imagePath << std::endl;
            }

        }
    }
}

int main() {
    using namespace std;

    std::map<std::string, int> params;
    read_params_from_file("C:\\Users\\zjc\\Desktop\\config\\template_color_config.txt", params);
    // Initialize MIL application
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
        M_NULL);

    // Load input image

    std::string folderPath ="C:/Users/zjc/Desktop/cotton_image_new"; // 请替换为你自己的文件夹路径
    // 加载文件夹中的所有图片
    LoadImagesFromFolder(folderPath);

    // Display result
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    return 0;
}