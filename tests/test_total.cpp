#include <windows.h>
#include <iostream>
#include <map>
#include <string>
#include <CVDL/OnnxRunner.h>

#include "Mil.h"
#include "Matrox/utils.h"
#include "Matrox/color_range.h"
#include "Matrox/template_matching.h"

// 宏定义
#define SAVE_PATH3 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")
#define SAVE_PATH4 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect2.png")
#define IMAGE_PATH MIL_TEXT(".\\test_imgs\\357.bmp")
std::string imagePath = "C:\\Users\\zjc\\Desktop\\dimo.bmp";

MIL_ID MilApplication, MilSystem, MilDisplay;
std::map<std::string, int> params;

using namespace std;
int main() {
    // 初始化 MIL 应用
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL) ;
    // 读取图片
    MIL_ID MilImage=M_NULL, MilHighSatResult = M_NULL, MilTemplateMatchingResult = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
    if (MilImage == M_NULL) {
        std::cerr << "Error: Failed to restore image from " << IMAGE_PATH << std::endl;
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }
    Timer timer1, timer2;
    std::map<std::string, int> params;
    read_params_from_file("..\\config\\color_range_config.txt", params);
    read_params_from_file("..\\config\\template_color_config.txt", params);
    TemplateMatcher matcher(MilSystem, MilDisplay, params);
    matcher.LoadConfig("..\\config\\template_config.txt");
    ONNXRunner runner;
    runner.load("C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx");
    timer1.printElapsedTime("Load config and templates and models");
    cv::Mat deep_result, high_sat_result, template_result, total_result;
    cout << "Sequence running start:" << endl;
    for(int i = 0; i < 10; i++) {
        cout << i << endl;
        timer1.restart();
        timer2.restart();
        // 艳丽色彩检测
        high_sat_detect(MilImage, MilHighSatResult, params);
        timer1.printElapsedTime("High Sat Predict finished");
        // 模板匹配检测
        matcher.predict(MilImage, MilTemplateMatchingResult, params);
        timer1.printElapsedTime("Template Matching Predict finished");
        // 深度学习检测
        cv::Mat cv_input = mil2mat(MilImage);
        std::vector<Detection> result = runner.predict(cv_input);
        deep_result = runner.postProcess(result, cv_input);
        std::string savepath = "C:\\Users\\zjc\\Desktop\\suspect_mask2.png";
        cv::imwrite(savepath, deep_result);
        timer1.printElapsedTime("Deep Learning Predict finished");
        high_sat_result = mil2mat(MilHighSatResult);
        template_result = mil2mat(MilTemplateMatchingResult);

        if (!deep_result.empty() && !high_sat_result.empty() && !template_result.empty()) {
            cv::bitwise_and(deep_result, high_sat_result, total_result);
            cv::bitwise_and(total_result, template_result, total_result);
        } else {
            cerr << "Error: One or more detection results are empty!" << endl;
        }
        timer2.printElapsedTime("Prediction finished Total");

        cv::imshow("Result", total_result);
        cv::waitKey(0);

    }



    // 释放资源
    MbufFree(MilImage);
    MbufFree(MilHighSatResult);
    MbufFree(MilTemplateMatchingResult);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    std::cout << "所有模块检测已完成！按 <Enter> 退出。" << std::endl;
    getchar();

    return 0;
}
