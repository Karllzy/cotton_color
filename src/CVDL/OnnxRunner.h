#ifndef ONNXRUNNER_H
#define ONNXRUNNER_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <iostream>
#include <vector>
#include <chrono>

const float CONFIDENCE_THRESHOLD = 0.2;
const float NMS_THRESHOLD = 0.2;
const int INPUT_WIDTH = 640;
const int INPUT_HEIGHT = 640;

struct Detection {
    cv::Rect box;
    float confidence;
};

// Class to measure elapsed time
class Timer {
public:
    Timer();
    void restart();
    void printElapsedTime(const std::string& message);

private:
    std::chrono::high_resolution_clock::time_point start_time;
};
class ONNXRunner {
public:
    ONNXRunner() = default; // Default constructor
    void load(const std::string& modelPath); // Load the model
    std::vector<Detection> predict(const cv::Mat& image); // Predict and generate results
    cv::Mat postProcess(const std::vector<Detection> &detections, const cv::Mat &image) const;

private:
    cv::dnn::Net net;
    int pad_top, pad_left;
    float scale;

    // Function prototypes
    static cv::Mat resizeAndPad(const cv::Mat& image, int targetWidth, int targetHeight, int& padTop, int& padLeft, float& scale, const cv::Scalar& padColor);
    static cv::Mat createDetectionMask(const cv::Mat& originalImage, const std::vector<Detection>& detections, float scale, int padTop, int padLeft);
    static cv::dnn::Net loadModel(const std::string& modelPath);
    cv::Mat preprocessImage(const cv::Mat& image, cv::dnn::Net& net, int& padTop, int& padLeft, float& scale) const;
    std::vector<Detection> performInference(cv::dnn::Net& net, const cv::Mat& inputImage) const;
    std::vector<Detection> applyNMS(std::vector<Detection>& detections) const;
    // Constants
    const float CONFIDENCE_THRESHOLD = 0.2f;
    const float NMS_THRESHOLD = 0.2f;
    const int INPUT_WIDTH = 640;
    const int INPUT_HEIGHT = 640;
};


#endif // ONNXRUNNER_H
