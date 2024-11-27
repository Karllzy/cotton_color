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
#include <filesystem> // For directory traversal

// 宏定义
#define SAVE_PATH3 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")
#define SAVE_PATH4 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect2.png")
#define IMAGE_DIR MIL_TEXT(".\\test_imgs\\cotton_image_new") // 文件夹路径

#define run_high_sat true;
#define run_templating true;
#define run_deep_learning true;

MIL_ID MilApplication, MilSystem, MilDisplay;
std::map<std::string, int> params;

namespace fs = std::filesystem; // 使用命名空间 fs 来方便调用

using namespace std;

cv::Mat overlayResultOnInput(const cv::Mat& cv_input, const cv::Mat& total_result, double alpha = 0.5, int colormap = cv::COLORMAP_JET) {
    // 1. 将 total_result 转换为伪彩色图像
    cv::Mat total_result_color;
    cv::applyColorMap(total_result, total_result_color, colormap); // 使用 JET 色图

    // 2. 确保 cv_input 是三通道图像（如果是灰度图像，则转换为 BGR）
    cv::Mat cv_input_rgb;
    if (cv_input.channels() == 1) {
        cv::cvtColor(cv_input, cv_input_rgb, cv::COLOR_GRAY2BGR);
    } else {
        cv_input_rgb = cv_input.clone(); // 保证不修改原始图像
    }

    // 3. 设置叠加透明度（alpha: 0.0-1.0）
    double beta = 1.0 - alpha;

    // 4. 使用加权和将 total_result_color 叠加到 cv_input_rgb 上
    cv::Mat overlay;
    cv::addWeighted(cv_input_rgb, alpha, total_result_color, beta, 0.0, overlay);

    // 5. 返回叠加后的图像
    return overlay;
}

int main() {
    // 初始化 MIL 应用
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

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

    // 遍历文件夹中的所有图片文件
    for (const auto& entry : fs::directory_iterator(IMAGE_DIR)) {
        if (entry.is_regular_file()) {
            string image_path = entry.path().string();
            cout << "Processing image: " << image_path << endl;

            // 读取当前图片
            MIL_ID MilImage = M_NULL, MilHighSatResult = M_NULL, MilTemplateMatchingResult = M_NULL;
            MbufRestore(convert_to_wstring(image_path), MilSystem, &MilImage);

            timer1.restart();
            timer2.restart();
            cv::Mat deep_result, high_sat_result, template_result, total_result;

            // 艳丽色彩检测
#if run_high_sat
            high_sat_detect(MilImage, MilHighSatResult, params);
            MIL_ID MilHighSatUint8 = convert_to_uint8(MilHighSatResult);
            // MdispSelect(MilDisplay, MilHighSatUint8);
            high_sat_result = mil2mat(MilHighSatResult);
            timer1.printElapsedTime("High Sat finished");
#else
            high_sat_result = cv::Mat::zeros(1024, 4096, CV_8UC1);
#endif

#if run_templating
            // 模板匹配检测
            matcher.predict(MilImage, MilTemplateMatchingResult, params);
            template_result = mil2mat(MilTemplateMatchingResult);
            timer1.printElapsedTime("Template Matching finished");
#else
            template_result = cv::Mat::zeros(1024, 4096, CV_8UC1);
#endif

#if run_deep_learning
            // 深度学习检测
            cv::Mat cv_input = mil2mat(MilImage);
            std::vector<Detection> result = runner.predict(cv_input);
            deep_result = runner.postProcess(result, cv_input);
#else
            deep_result = cv::Mat::zeros(1024, 4096, CV_8UC1);
#endif

            timer1.printElapsedTime("Deep Learning finished");

            if (!deep_result.empty() && !high_sat_result.empty() && !template_result.empty()) {
                cv::bitwise_or(deep_result, high_sat_result, total_result);
                cv::bitwise_or(total_result, template_result, total_result);
            } else {
                cerr << "Error: One or more detection results are empty!" << endl;
            }

            timer2.printElapsedTime("Prediction finished Total");

            // 保存结果
            cv::imwrite("./runs/deep_result_" + entry.path().filename().string() + ".png", deep_result);
            cv::imwrite("./runs/high_sat_result_" + entry.path().filename().string() + ".png", high_sat_result);
            cv::imwrite("./runs/template_result_" + entry.path().filename().string() + ".png", template_result);
            cv::imwrite("./runs/total_result_" + entry.path().filename().string() + ".png", total_result);

            std::vector<cv::Mat> images = { deep_result, high_sat_result, template_result, total_result };
            displayCombinedResults(images, "Combined Results");
            images = { cv_input, overlayResultOnInput(cv_input, total_result, 0.1, cv::COLORMAP_OCEAN)};
            displayCombinedResults(images, "Combined Results");
            // 释放当前图片资源
            MbufFree(MilImage);
            MbufFree(MilHighSatResult);
            MbufFree(MilTemplateMatchingResult);
        }
    }

#if run_templating
    matcher.~TemplateMatcher();
#endif
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
    return 0;
}
