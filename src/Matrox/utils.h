//
// Created by zjc on 24-11-18.
//

#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <iostream>
#include <vector>
#include <Mil.h>
#include <fstream>
#include <string>
#include <map>
#include"opencv2/opencv.hpp"
#include <sstream>

// 声明全局变量（注意：这里只是声明，不是定义）
extern __int64 MilApplication;
extern __int64 MilSystem;
extern __int64 MilDisplay;

template <typename Func>
// Time measurement function
void measure_execution_time(Func func) {
    std::chrono::time_point<std::chrono::steady_clock> start;
    start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Function execution time: " << duration.count() << " milliseconds" << std::endl;
}

std::vector<int> psLabToOpenCVLab(const std::vector<int>& lab_ps);

std::vector<int> opencvLabToPsLab(const std::vector<int>& lab_cv);

void convert_to_uint8(const MIL_ID& input_img, MIL_ID& output_img);

std::wstring convert_to_wstring(const std::string& str);

void read_params_from_file(const std::string& filename, std::map<std::string, int>& params) ;

cv::Mat mil2mat(MIL_ID mil_img);

std::unordered_map<std::string, int> loadConfig(const std::string& filename);

void displayCombinedResults(const std::vector<cv::Mat>& images, const std::string& windowName);
#endif //UTILS_H
