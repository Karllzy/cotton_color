// #include "vector"
// #include"iostream"
// #include"string"
// #include"Matrox/utils.h"
// #include"opencv2/opencv.hpp"
//
// #define IMAGE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\8.bmp")
// #define SAVE_PATH MIL_TEXT("C:\\Users\\zjc\\Desktop\\suspect.png")

// int main() {
//     MIL_ID MilImage = M_NULL;
//     MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;
//
//     MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL,
//       M_NULL);
//     MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
//     cv::Mat opencvImage = milToMat(MilImage);
//     imshow("opencv", opencvImage);
//     // MosGetch();
//
//     return 0;
// }
// int main() {
//     MIL_ID MilApplication = M_NULL, MilSystem = M_NULL, MilDisplay = M_NULL;
//     MIL_ID MilImage = M_NULL;
//
//     // 初始化 MIL 应用程序、系统和显示
//     MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);
//     if (MilApplication == M_NULL || MilSystem == M_NULL || MilDisplay == M_NULL) {
//         std::cerr << "MIL Initialization failed!" << std::endl;
//         return -1;
//     }
//
//     // 加载图像
//     MbufRestore(IMAGE_PATH, MilSystem, &MilImage);
//     if (MilImage == M_NULL) {
//         std::cerr << "Failed to load MIL image!" << std::endl;
//         MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL,M_NULL);
//         return -1;
//     }
//
//     // 转换并显示
//     cv::Mat opencvImage = milToMat(MilImage);
//     if (!opencvImage.empty()) {
//         cv::imshow("opencv", opencvImage);
//         cv::waitKey(0);
//     }
//     std:: string savepath="C:\\Users\\zjc\\Desktop\\suspect.png";
//     cv::imwrite(savepath,opencvImage);
//     // 释放资源
//     MbufFree(MilImage);
//     MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL,M_NULL);  // 使用 MappFreeDefault 代替 MappFree
//
//     return 0;
// }

// int main() {
//     cv::Mat img = cv::imread("C:\\Users\\zjc\\Desktop\\suspect.png");
//     if (img.empty()) {
//         std::cout << "图像加载失败!" << std::endl;
//         return -1;
//     }
//
//     // 处理图像
//     processImage(img);
// }

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

    cv::imshow("Detection Mask", detectionMask);

    // Save the result

    std::string savepath = "C:\\Users\\zjc\\Desktop\\suspect_mask.png";
    cv::imwrite(savepath, detectionMask);
    timer1.printElapsedTime("Time to run inference");

    cv::waitKey(0);
    return 0;
}
