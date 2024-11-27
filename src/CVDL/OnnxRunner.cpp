#include "OnnxRunner.h"

// Timer class implementation
Timer::Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

void Timer::restart() {
    start_time = std::chrono::high_resolution_clock::now();
}

void Timer::printElapsedTime(const std::string& message) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << message << ": " << elapsed.count() << " seconds" << std::endl;
    start_time = end_time;
}

// Resize and pad input image
cv::Mat ONNXRunner::resizeAndPad(const cv::Mat& image, int targetWidth, int targetHeight, int& padTop, int& padLeft, float& scale, const cv::Scalar& padColor) {
    int originalWidth = image.cols;
    int originalHeight = image.rows;

    scale = std::min((float)targetWidth / originalWidth, (float)targetHeight / originalHeight);
    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));

    padTop = (targetHeight - newHeight) / 2;
    int padBottom = targetHeight - newHeight - padTop;
    padLeft = (targetWidth - newWidth) / 2;
    int padRight = targetWidth - newWidth - padLeft;

    cv::Mat paddedImage;
    cv::copyMakeBorder(resizedImage, paddedImage, padTop, padBottom, padLeft, padRight, cv::BORDER_CONSTANT, padColor);

    return paddedImage;
}

// Create detection mask
cv::Mat ONNXRunner::createDetectionMask(const cv::Mat& originalImage, const std::vector<Detection>& detections, float scale, int padTop, int padLeft) {
    cv::Mat mask = cv::Mat::zeros(originalImage.size(), CV_8UC1);  // Single channel mask

    for (const auto& detection : detections) {
        int x = static_cast<int>((detection.box.x - padLeft) / scale);
        int y = static_cast<int>((detection.box.y - padTop) / scale);
        int w = static_cast<int>(detection.box.width / scale);
        int h = static_cast<int>(detection.box.height / scale);

        x = std::max(0, std::min(x, originalImage.cols - 1));
        y = std::max(0, std::min(y, originalImage.rows - 1));
        w = std::min(w, originalImage.cols - x);
        h = std::min(h, originalImage.rows - y);

        cv::rectangle(mask, cv::Rect(x, y, w, h), cv::Scalar(255), cv::FILLED);  // White color for detections
    }

    return mask;
}

// Load the ONNX model
cv::dnn::Net ONNXRunner::loadModel(const std::string& modelPath) {
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);  // Use CUDA backend
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);    // Run on GPU
    return net;
}

// Preprocess image for model input
cv::Mat ONNXRunner::preprocessImage(const cv::Mat& image, cv::dnn::Net& net, int& padTop, int& padLeft, float& scale) const {
    cv::Scalar padColor(128, 128, 128);  // Gray padding
    cv::Mat inputImage = resizeAndPad(image, INPUT_WIDTH, INPUT_HEIGHT, padTop, padLeft, scale, padColor);
    cv::Mat blob = cv::dnn::blobFromImage(inputImage, 1 / 255.0, cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    return inputImage;
}

// Perform inference on the input image
std::vector<Detection> ONNXRunner::performInference(cv::dnn::Net& net, const cv::Mat& inputImage) const {
    std::vector<Detection> detections;
    cv::Mat output = net.forward();
    float* data = (float*)output.data;

    for (int i = 0; i < 25200; ++i) {
        float confidence = data[i * 6 + 4];
        if (confidence >= CONFIDENCE_THRESHOLD) {
            float cx = data[i * 6];
            float cy = data[i * 6 + 1];
            float w = data[i * 6 + 2];
            float h = data[i * 6 + 3];

            cx = cx * inputImage.cols / INPUT_WIDTH;
            cy = cy * inputImage.rows / INPUT_HEIGHT;
            w = w * inputImage.cols / INPUT_WIDTH;
            h = h * inputImage.rows / INPUT_HEIGHT;

            int left = static_cast<int>(cx - w / 2);
            int top = static_cast<int>(cy - h / 2);
            int width = static_cast<int>(w);
            int height = static_cast<int>(h);

            left = std::max(0, std::min(left, inputImage.cols - 1));
            top = std::max(0, std::min(top, inputImage.rows - 1));
            width = std::min(width, inputImage.cols - left);
            height = std::min(height, inputImage.rows - top);

            detections.push_back({cv::Rect(left, top, width, height), confidence});
        }
    }

    return detections;
}

// Apply Non-Maximum Suppression
std::vector<Detection> ONNXRunner::applyNMS(std::vector<Detection>& detections) const {
    std::vector<int> indices;
    std::vector<cv::Rect> boxes;
    std::vector<float> scores;

    for (const auto& detection : detections) {
        boxes.push_back(detection.box);
        scores.push_back(detection.confidence);
    }

    cv::dnn::NMSBoxes(boxes, scores, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);
    std::vector<Detection> finalDetections;
    for (int idx : indices) {
        finalDetections.push_back(detections[idx]);
    }

    return finalDetections;
}


void ONNXRunner::load(const std::string& modelPath) {
    this->net = ONNXRunner::loadModel(modelPath);
    // 创建一个空的输入矩阵作为预热数据（假定模型输入是 RGB 图像）
    cv::Mat dummyInput = cv::Mat::zeros(INPUT_HEIGHT, INPUT_WIDTH, CV_8UC3); // 640x640 的全零矩阵

    // 调用 predict 方法进行预热
    this->predict(dummyInput);
}; // Load the model

std::vector<Detection> ONNXRunner::predict(const cv::Mat &image) {
    cv::dnn::Net net = this->net;

    cv::Mat inputImage = preprocessImage(image, net, this->pad_top, this->pad_left, this->scale);

    // Perform inference
    std::vector<Detection> detections = performInference(net, inputImage);
    // Apply Non-Maximum Suppression
    std::vector<Detection> finalDetections = applyNMS(detections);
    return finalDetections;
}

cv::Mat ONNXRunner::postProcess(const std::vector<Detection>& detections, const cv::Mat& image) const {
    // Create and show the detection mask
    cv::Mat detectionMask = ONNXRunner::createDetectionMask(image, detections, this->scale, this->pad_top, this->pad_left);
    return detectionMask;
}