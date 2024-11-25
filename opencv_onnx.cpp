//
// Created by zjc on 24-11-19.
//
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
class Timer {
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    // 重新启动定时器
    void restart() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    // 获取并打印从上次启动到当前的时间差
    void printElapsedTime(const std::string& message) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        std::cout << message << ": " << elapsed.count() << " seconds" << std::endl;
        // 重新启动定时器以供下次测量
        start_time = end_time;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};


// 在图像上绘制检测框
void drawDetections(cv::Mat& inputImage, const std::vector<Detection>& detections) {
    for (const auto& detection : detections) {
        cv::rectangle(inputImage, detection.box, cv::Scalar(0, 255, 0), 2);
        std::string label = "Object: " + cv::format("%.2f", detection.confidence);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        cv::rectangle(inputImage, cv::Point(detection.box.x, detection.box.y - labelSize.height - baseLine),
                      cv::Point(detection.box.x + labelSize.width, detection.box.y), cv::Scalar(0, 255, 0), cv::FILLED);
        cv::putText(inputImage, label, cv::Point(detection.box.x, detection.box.y - baseLine), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
}
cv::Mat resizeAndPad(const cv::Mat& image, int targetWidth, int targetHeight, int& padTop, int& padLeft, float& scale, const cv::Scalar& padColor) {
    int originalWidth = image.cols;
    int originalHeight = image.rows;

    // 计算缩放比例
    scale = std::min((float)targetWidth / originalWidth, (float)targetHeight / originalHeight);

    // 缩放后的新尺寸
    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    // 缩放图像
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));

    // 计算填充值
    padTop = (targetHeight - newHeight) / 2;
    int padBottom = targetHeight - newHeight - padTop;
    padLeft = (targetWidth - newWidth) / 2;
    int padRight = targetWidth - newWidth - padLeft;

    // 在图像周围添加填充，使用灰色 (128, 128, 128) 填充
    cv::Mat paddedImage;
    cv::copyMakeBorder(resizedImage, paddedImage, padTop, padBottom, padLeft, padRight, cv::BORDER_CONSTANT, padColor);

    return paddedImage;
}
int main() {
    // 模型路径和图片路径
    std::string modelPath = "C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx";
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\dimo.bmp";
    Timer timer1;
    // 加载模型
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA); // 设置为使用 CUDA 后端
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);   // 设置为在 GPU 上运行
    timer1.printElapsedTime("Time to load the model");
    // 读取输入图像

    timer1.restart();
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Could not read the image: " << imagePath << std::endl;
        return -1;
    }
    // 设置填充颜色为灰色
    cv::Scalar padColor(128, 128, 128);

    // 预处理图像并添加填充
    int padTop, padLeft;
    float scale;
    cv::Mat inputImage = resizeAndPad(image, INPUT_WIDTH, INPUT_HEIGHT, padTop, padLeft, scale, padColor);

    // 显示调整和填充后的图像
    // cv::imshow("Resized and Padded Image", inputImage);
    // 预处理图像
    cv::Mat blob = cv::dnn::blobFromImage(inputImage, 1 / 255.0, cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    timer1.printElapsedTime("Time to preprocessing");
    timer1.restart();
    for(int j = 0; j <30; j++) {
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
                cx = cx * inputImage.cols / INPUT_WIDTH;
                cy = cy * inputImage.rows / INPUT_HEIGHT;
                w = w * inputImage.cols / INPUT_WIDTH;
                h = h * inputImage.rows / INPUT_HEIGHT;

                // Proceed with the rest of your code
                int left = static_cast<int>(cx - w / 2);
                int top = static_cast<int>(cy - h / 2);
                int width = static_cast<int>(w);
                int height = static_cast<int>(h);

                // Ensure coordinates are within image bounds
                left = std::max(0, std::min(left, inputImage.cols - 1));
                top = std::max(0, std::min(top, inputImage.rows - 1));
                width = std::min(width, inputImage.cols - left);
                height = std::min(height, inputImage.rows - top);

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
            drawDetections(inputImage, {detection});
        }

        std::vector<Detection> finalDetections;
        for (int idx : indices) {
            finalDetections.push_back(detections[idx]);
        }
        for (int i = 0; i < 25200; ++i) {
            float confidence = data[i * 6 + 4];
            if (confidence >= CONFIDENCE_THRESHOLD) {
                // std::cout << "Detection " << i << ": confidence=" << confidence << std::endl;
            }
        }

        // 绘制检测框并显示图像
        drawDetections(image, finalDetections);
        timer1.printElapsedTime("Time to run inference");
    }
    cv::imshow("Detections", inputImage);
    cv::waitKey(0);

    return 0;
}
//