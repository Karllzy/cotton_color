#include <windows.h>
#include <iostream>
#include <map>
#include <string>
#include <CVDL/OnnxRunner.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "Mil.h"
#include "Matrox/utils.h"
#include "Matrox/color_range.h"
#include "Matrox/template_matching.h"

// 宏定义
#define SAVE_PATH3 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")
#define SAVE_PATH4 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect2.png")
#define IMAGE_PATH MIL_TEXT(".\\test_imgs\\dimo.bmp")

#define run_high_sat true;
#define run_templating true;
#define run_deep_learning true;

MIL_ID MilApplication, MilSystem, MilDisplay;
std::map<std::string, int> params;

using namespace std;
int main() {
    // 初始化 MIL 应用
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL) ;
    // 读取图片
    MIL_ID MilImage=M_NULL, MilHighSatResult = M_NULL, MilTemplateMatchingResult = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
    Timer timer1, timer2;
    std::map<std::string, int> params;
    read_params_from_file("..\\config\\color_range_config.txt", params);
    read_params_from_file("..\\config\\template_color_config.txt", params);
#if run_templating
        TemplateMatcher matcher(MilSystem, MilDisplay, params);
        matcher.LoadConfig("..\\config\\template_config.txt");
#endif

#if run_deep_learning
    ONNXRunner runner;
    runner.load("C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx");
#endif

    timer1.printElapsedTime("Load config and templates and models");

    cout << "Sequence running start:" << endl;
    for(int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cout << i << endl;
        timer1.restart();
        timer2.restart();
        cv::Mat deep_result, high_sat_result, template_result, total_result;

        // 艳丽色彩检测
#if run_high_sat
        high_sat_detect(MilImage, MilHighSatResult, params);
        MIL_ID MilHighSatUint8 = convert_to_uint8(MilHighSatResult);
        MdispSelect(MilDisplay, MilHighSatUint8);
        high_sat_result = mil2mat(MilHighSatResult);
        timer1.printElapsedTime("High Sat finished");
#else
        high_sat_result = cv::Mat::zeros(1024,4096, CV_8UC1);
#endif

#if run_templating
        // 模板匹配检测
        matcher.predict(MilImage, MilTemplateMatchingResult, params);
        template_result = mil2mat(MilTemplateMatchingResult);
        timer1.printElapsedTime("Template Matching finished");
#else
        template_result= cv::Mat::zeros(1024,4096, CV_8UC1);
#endif
#if run_deep_learning
        // 深度学习检测
        cv::Mat cv_input = mil2mat(MilImage);
        std::vector<Detection> result = runner.predict(cv_input);
        deep_result = runner.postProcess(result, cv_input);
         // 640x640 的全零矩阵
#else
        deep_result = cv::Mat::zeros(1024,4096, CV_8UC1);
#endif



        timer1.printElapsedTime("Deep Learning finished");


        if (!deep_result.empty() && !high_sat_result.empty() && !template_result.empty()) {
            cv::bitwise_or(deep_result, high_sat_result, total_result);
            cv::bitwise_or(total_result, template_result, total_result);
        } else {
            cerr << "Error: One or more detection results are empty!" << endl;
        }
        timer2.printElapsedTime("Prediction finished Total");


        cv::imwrite("./runs/deep_result"+std::to_string(i)+".png", deep_result);
        cv::imwrite("./runs/high_sat_result"+std::to_string(i)+".png", high_sat_result);
        cv::imwrite("./runs/template_result"+std::to_string(i)+".png", template_result);
        cv::imwrite("./runs/total_result"+std::to_string(i)+".png", total_result);
        std::vector<cv::Mat> images = {deep_result, high_sat_result, template_result, total_result};
        displayCombinedResults(images, "Combined Results");

    }
    // 释放资源
    MbufFree(MilImage);
    MbufFree(MilHighSatResult);
    MbufFree(MilTemplateMatchingResult);
#if run_templating
    matcher.~TemplateMatcher();
#endif
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    std::cout << "所有模块检测已完成！按 <Enter> 退出。" << std::endl;
    getchar();

    return 0;
}
