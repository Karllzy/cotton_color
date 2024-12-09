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
// #define SAVE_PATH3 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")
// #define SAVE_PATH4 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect2.png")
#define IMAGE_DIR MIL_TEXT("C:\\Users\\ZLSDKJ\\Desktop\\iamge\\1.bmp") // 文件夹路径

#define run_high_sat true;
#define run_templating false;
#define run_deep_learning false;

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
        cv::cvtColor(cv_input, cv_input_rgb, cv::COLORMAP_WINTER);
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
void process_iterations(MIL_ID MilImage, MIL_ID MilHighSatResult, const std::map<std::string, int>& params, int iterations, const std::string& thread_name) {
    Timer timer;
    for(int i = 0; i < iterations; ++i) {
        timer.restart();
        high_sat_detect(MilImage, MilHighSatResult, params);
        timer.printElapsedTime(thread_name + " - Iteration " + std::to_string(i+1) + " finished");
    }
}

int main() {
    // 初始化 MIL 应用
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

    Timer timer1, timer2;
    std::map<std::string, int> params;
    read_params_from_file("C:\\Users\\ZLSDKJ\\Desktop\\color_range_config.txt", params);
    // read_params_from_file("..\\config\\template_color_config.txt", params);
    MIL_ID MilImage = M_NULL, MilHighSatResult = M_NULL, MilTemplateMatchingResult = M_NULL;
    MbufRestore(IMAGE_DIR, MilSystem, &MilImage);
    const int iterations_per_thread = 500;

    // 创建两个线程
    std::thread thread1(process_iterations, MilImage, MilHighSatResult, std::cref(params), iterations_per_thread, "Thread 1");
    std::thread thread2(process_iterations, MilImage, MilHighSatResult, std::cref(params), iterations_per_thread, "Thread 2");

    // 等待两个线程完成
    thread1.join();
    thread2.join();

    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
    return 0;
}
