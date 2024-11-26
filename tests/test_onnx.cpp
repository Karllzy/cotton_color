
#include "CVDL/OnnxRunner.h"

int main() {
    std::string modelPath = "C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx";
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\dimo.bmp";
    Timer timer1;

    // Load the model
    cv::dnn::Net net = loadModel(modelPath);
    timer1.printElapsedTime("Time to load the model");

    // Read the input image
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Could not read the image: " << imagePath << std::endl;
        return -1;
    }

    // Preprocess image
    int padTop, padLeft;
    float scale;
    cv::Mat inputImage = preprocessImage(image, net, padTop, padLeft, scale);
    timer1.printElapsedTime("Time to preprocess image");

    // Perform inference
    std::vector<Detection> detections = performInference(net, inputImage);

    // Apply Non-Maximum Suppression
    std::vector<Detection> finalDetections = applyNMS(detections);
    std::cout << "Number of detections after NMS: " << finalDetections.size() << std::endl;

    // Create and show the detection mask
    cv::Mat detectionMask = createDetectionMask(image, finalDetections, scale, padTop, padLeft);

    // cv::imshow("Detection Mask", detectionMask);

    // Save the result

    std::string savepath = "C:\\Users\\zjc\\Desktop\\suspect_mask.png";
    cv::imwrite(savepath, detectionMask);
    timer1.printElapsedTime("Time to run inference");

    cv::waitKey(0);
    return 0;
}
