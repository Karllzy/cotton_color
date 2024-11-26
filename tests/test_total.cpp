#include <windows.h>
#include <iostream>
#include <map>
#include <string>
#include "Mil.h"
#include "Matrox/utils.h"
#include "Matrox/color_range.h"
#include "Matrox/template_matching.h"

// 宏定义
#define SAVE_PATH3 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")
#define SAVE_PATH4 MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect2.png")
#define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\cotton_image_new\\357.bmp")

MIL_ID MilApplication, MilSystem, MilDisplay;
std::map<std::string, int> params;

void run_high_sat_detect(MIL_ID MilImage, std::map<std::string, int>& params) {
    MIL_ID detection_result = M_NULL;
    read_params_from_file("C:\\Users\\zjc\\Desktop\\config\\color_range_config.txt", params);

    measure_execution_time([&]() {
        high_sat_detect(MilImage, detection_result, params);
    });

    MbufSave(SAVE_PATH3, detection_result);
    MbufFree(detection_result);
}

void run_pre_process(MIL_ID MilImage, std::map<std::string, int>& params) {
    MIL_ID detection_result = M_NULL;
    MIL_ID detection_resize = M_NULL;
    MIL_ID output_Image = M_NULL;

    read_params_from_file("C:\\Users\\zjc\\Desktop\\config\\template_color_config.txt", params);
    TemplateMatcher matcher(MilSystem, MilDisplay, params);

    measure_execution_time([&]() {
        pre_process(MilImage, detection_result, params);
        MbufAlloc2d(MilSystem, MbufInquire(detection_result, M_SIZE_X, M_NULL) / 2,
                    MbufInquire(detection_result, M_SIZE_Y, M_NULL) / 2, 1 + M_UNSIGNED,
                    M_IMAGE + M_PROC, &detection_resize);
        MimResize(detection_result, detection_resize, 0.5, 0.5, M_DEFAULT);

        matcher.LoadTemplate(params);
        matcher.FindTemplates(detection_resize, output_Image, params);
    });

    MbufSave(SAVE_PATH4, detection_result);
    MbufFree(detection_resize);
    MbufFree(output_Image);
    MbufFree(detection_result);
}

int main() {
    // 初始化 MIL 应用

    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL) ;
    // 读取图片
    MIL_ID MilImage = M_NULL;
    MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
    if (MilImage == M_NULL) {
        std::cerr << "Error: Failed to restore image from " << IMAGE_PATH << std::endl;
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }

    // 加载参数
    // 创建两个子进程
    PROCESS_INFORMATION processInfo1, processInfo2;
    STARTUPINFO startupInfo1, startupInfo2;

    ZeroMemory(&startupInfo1, sizeof(startupInfo1));
    ZeroMemory(&startupInfo2, sizeof(startupInfo2));
    ZeroMemory(&processInfo1, sizeof(processInfo1));
    ZeroMemory(&processInfo2, sizeof(processInfo2));

    startupInfo1.cb = sizeof(startupInfo1);
    startupInfo2.cb = sizeof(startupInfo2);

    // 子进程1
    if (!CreateProcess(NULL, LPWSTR((LPSTR) "ChildProcess1.exe"), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo1, &processInfo1)) {
        std::cerr << "Error: Failed to create process 1. Error code: " << GetLastError() << std::endl;
        MbufFree(MilImage);
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }

    // 子进程2
    if (!CreateProcess(NULL, LPWSTR((LPSTR) "ChildProcess2.exe"), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo2, &processInfo2)) {
        std::cerr << "Error: Failed to create process 2. Error code: " << GetLastError() << std::endl;
        MbufFree(MilImage);
        MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
        return -1;
    }

    // 等待两个子进程完成
    WaitForSingleObject(processInfo1.hProcess, INFINITE);
    WaitForSingleObject(processInfo2.hProcess, INFINITE);

    CloseHandle(processInfo1.hProcess);
    CloseHandle(processInfo1.hThread);
    CloseHandle(processInfo2.hProcess);
    CloseHandle(processInfo2.hThread);

    // 释放资源
    MbufFree(MilImage);
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    std::cout << "所有模块检测已完成！按 <Enter> 退出。" << std::endl;
    getchar();

    return 0;
}
