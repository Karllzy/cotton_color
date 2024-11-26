#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <iostream>

// Parameters
const float CONFIDENCE_THRESHOLD = 0.2; // Confidence threshold
const float NMS_THRESHOLD = 0.2;       // Non-maximum suppression threshold
const int INPUT_WIDTH = 640;            // Model input width
const int INPUT_HEIGHT = 640;           // Model input height

// Detection structure
struct Detection {
    cv::Rect box;
    float confidence;
};

class Timer {
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    // Restart the timer
    void restart() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    // Get and print the time elapsed since last start
    void printElapsedTime(const std::string& message) {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        std::cout << message << ": " << elapsed.count() << " seconds" << std::endl;
        // Restart the timer for the next measurement
        start_time = end_time;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};

// Function to resize and pad the input image
cv::Mat resizeAndPad(const cv::Mat& image, int targetWidth, int targetHeight, int& padTop, int& padLeft, float& scale, const cv::Scalar& padColor) {
    int originalWidth = image.cols;
    int originalHeight = image.rows;

    // Calculate scaling factor
    scale = std::min((float)targetWidth / originalWidth, (float)targetHeight / originalHeight);

    // New dimensions after scaling
    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    // Resize the image
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));

    // Padding calculations
    padTop = (targetHeight - newHeight) / 2;
    int padBottom = targetHeight - newHeight - padTop;
    padLeft = (targetWidth - newWidth) / 2;
    int padRight = targetWidth - newWidth - padLeft;

    // Add padding around the image (using gray color)
    cv::Mat paddedImage;
    cv::copyMakeBorder(resizedImage, paddedImage, padTop, padBottom, padLeft, padRight, cv::BORDER_CONSTANT, padColor);

    return paddedImage;
}

// Function to create an image with only detected regions filled as white (rest black) in the original image size
cv::Mat createDetectionMask(const cv::Mat& originalImage, const std::vector<Detection>& detections, float scale, int padTop, int padLeft) {
    // Create a black image with the same size as the original image
    cv::Mat mask = cv::Mat::zeros(originalImage.size(), CV_8UC1);  // Single channel for black and white mask

    // Fill the detected regions with white
    for (const auto& detection : detections) {
        // Rescale the coordinates from the padded image back to the original image
        int x = static_cast<int>((detection.box.x - padLeft) / scale);
        int y = static_cast<int>((detection.box.y - padTop) / scale);
        int w = static_cast<int>(detection.box.width / scale);
        int h = static_cast<int>(detection.box.height / scale);

        // Ensure coordinates are within the bounds of the original image
        x = std::max(0, std::min(x, originalImage.cols - 1));
        y = std::max(0, std::min(y, originalImage.rows - 1));
        w = std::min(w, originalImage.cols - x);
        h = std::min(h, originalImage.rows - y);

        cv::rectangle(mask, cv::Rect(x, y, w, h), cv::Scalar(255), cv::FILLED);  // White color for detections
    }

    return mask;
}

int main() {
    // Model and image paths
    std::string modelPath = "C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx";
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\dimo.bmp";
    Timer timer1;

    // Load the model
    cv::dnn::Net net = cv::dnn::readNetFromONNX(modelPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA); // Use CUDA backend
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);   // Run on GPU
    timer1.printElapsedTime("Time to load the model");

    // Read the input image
    timer1.restart();
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Could not read the image: " << imagePath << std::endl;
        return -1;
    }

    // Set padding color (gray)
    cv::Scalar padColor(128, 128, 128);

    // Preprocess image and add padding
    int padTop, padLeft;
    float scale;
    cv::Mat inputImage = resizeAndPad(image, INPUT_WIDTH, INPUT_HEIGHT, padTop, padLeft, scale, padColor);

    // Prepare image for model input
    cv::Mat blob = cv::dnn::blobFromImage(inputImage, 1 / 255.0, cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);

    timer1.printElapsedTime("Time to preprocess image");

    for (int j = 0; j < 1; j++) {
        // Run inference
        cv::Mat output = net.forward();

        // Process output data
        std::vector<Detection> detections;
        float* data = (float*)output.data;

        for (int i = 0; i < 25200; ++i) {
            float confidence = data[i * 6 + 4]; // Confidence score
            if (confidence >= CONFIDENCE_THRESHOLD) {
                // Get bounding box coordinates
                float cx = data[i * 6];
                float cy = data[i * 6 + 1];
                float w = data[i * 6 + 2];
                float h = data[i * 6 + 3];

                // Map to image coordinates
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

                // Add detection to vector
                detections.push_back({cv::Rect(left, top, width, height), confidence});
            }
        }

        // Non-Maximum Suppression
        std::vector<int> indices;
        std::vector<cv::Rect> boxes;
        std::vector<float> scores;

        for (const auto& detection : detections) {
            boxes.push_back(detection.box);
            scores.push_back(detection.confidence);
        }

        cv::dnn::NMSBoxes(boxes, scores, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);
        std::cout << "Number of detections after NMS: " << indices.size() << std::endl;

        std::vector<Detection> finalDetections;
        for (int idx : indices) {
            finalDetections.push_back(detections[idx]);
        }

        // Create the mask for the detected regions (matching original image size)
        cv::Mat detectionMask = createDetectionMask(image, finalDetections, scale, padTop, padLeft);

        // Show the mask
        cv::imshow("Detection Mask", detectionMask);
        
        // Save the result as an image
        std::string savepath = "C:\\Users\\zjc\\Desktop\\suspect_mask.png";
        cv::imwrite(savepath, detectionMask);

        timer1.printElapsedTime("Time to run inference");
    }

    cv::waitKey(0);

    return 0;
}