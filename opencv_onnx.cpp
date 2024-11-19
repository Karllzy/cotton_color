#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <iostream>

// 参数
const float CONFIDENCE_THRESHOLD = 0.2; // 置信度阈值
const float NMS_THRESHOLD = 0.2;       // 非极大值抑制阈值
const int INPUT_WIDTH = 640;            // 模型输入宽度
const int INPUT_HEIGHT = 640;           // 模型输入高度

// 检测结构体
struct Detection {
    cv::Rect box;
    float confidence;
};

// 在图像上绘制检测框
void drawDetections(cv::Mat& image, const std::vector<Detection>& detections) {
    for (const auto& detection : detections) {
        cv::rectangle(image, detection.box, cv::Scalar(0, 255, 0), 2);
        std::string label = "Object: " + cv::format("%.2f", detection.confidence);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        cv::rectangle(image, cv::Point(detection.box.x, detection.box.y - labelSize.height - baseLine),
                      cv::Point(detection.box.x + labelSize.width, detection.box.y), cv::Scalar(0, 255, 0), cv::FILLED);
        cv::putText(image, label, cv::Point(detection.box.x, detection.box.y - baseLine), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
}

int main() {
    // 模型路径和图片路径
    std::string modelPath = "C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx";
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\yolo_resized_image_640x640.png";

    // 加载模型
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);

    // 读取输入图像
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Could not read the image: " << imagePath << std::endl;
        return -1;
    }

    // 预处理图像
    cv::Mat blob = cv::dnn::blobFromImage(image, 1 / 255.0, cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);


    // 推理模型
    cv::Mat output = net.forward();

    // 处理输出数据
    std::vector<Detection> detections;
    float* data = (float*)output.data;
    for (int i = 0; i < 25200; ++i) {
        float confidence = data[i * 6 + 4]; // 置信度
        if (confidence >= CONFIDENCE_THRESHOLD) {
            // 获取检测框并映射到图像坐标
            // Remove the unnecessary multiplication
            float cx = data[i * 6];
            float cy = data[i * 6 + 1];
            float w = data[i * 6 + 2];
            float h = data[i * 6 + 3];

            // If needed, adjust for differences between input image size and model input size
            // Since they are the same in your case, this step can be omitted or kept as is
            cx = cx * image.cols / INPUT_WIDTH;
            cy = cy * image.rows / INPUT_HEIGHT;
            w = w * image.cols / INPUT_WIDTH;
            h = h * image.rows / INPUT_HEIGHT;

            // Proceed with the rest of your code
            int left = static_cast<int>(cx - w / 2);
            int top = static_cast<int>(cy - h / 2);
            int width = static_cast<int>(w);
            int height = static_cast<int>(h);

            // Ensure coordinates are within image bounds
            left = std::max(0, std::min(left, image.cols - 1));
            top = std::max(0, std::min(top, image.rows - 1));
            width = std::min(width, image.cols - left);
            height = std::min(height, image.rows - top);

            // Add detection
            detections.push_back({cv::Rect(left, top, width, height), confidence});

        }
    }



    // 非极大值抑制
    std::vector<int> indices;
    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    for (const auto& detection : detections) {
        boxes.push_back(detection.box);

        scores.push_back(detection.confidence);
    }
    cv::dnn::NMSBoxes(boxes, scores, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);
    std::cout << "Number of detections after NMS: " << indices.size() << std::endl;
    if (indices.empty()) {
        std::cout << "No boxes passed NMS." << std::endl;
    }
    for (int idx : indices) {
        Detection detection = detections[idx];
        std::cout << "Drawing box at: (" << detection.box.x << ", " << detection.box.y
                  << "), width: " << detection.box.width << ", height: " << detection.box.height << std::endl;
        drawDetections(image, {detection});
    }

    std::vector<Detection> finalDetections;
    for (int idx : indices) {
        finalDetections.push_back(detections[idx]);
    }
    for (int i = 0; i < 25200; ++i) {
        float confidence = data[i * 6 + 4];
        if (confidence >= CONFIDENCE_THRESHOLD) {
            std::cout << "Detection " << i << ": confidence=" << confidence << std::endl;
        }
    }

    // 绘制检测框并显示图像
    drawDetections(image, finalDetections);
    cv::imshow("Detections", image);
    cv::waitKey(0);

    return 0;
}
//
// Created by zjc on 24-11-19.
//
