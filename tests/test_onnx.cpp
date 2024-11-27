
#include "CVDL/OnnxRunner.h"

int main() {
    std::string modelPath = "C:\\Users\\zjc\\Desktop\\dimo_11.14.onnx";
    std::string imagePath = "C:\\Users\\zjc\\Desktop\\dimo.bmp";
    Timer timer1;

    // Load the model
    ONNXRunner runner;
    runner.load(modelPath);
    timer1.printElapsedTime("Time to load the model");

    // Read the input image
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Could not read the image: " << imagePath << std::endl;
        return -1;
    }
    cv::Mat mask;
    timer1.printElapsedTime("Time to load image.");
    for(int i = 0; i < 10; i++) {
        std:: cout << "Run time: " << i << std::endl;
        std::vector<Detection> result = runner.predict(image);
        timer1.printElapsedTime("Time to predict result");
        mask = runner.postProcess(result, image);
        timer1.printElapsedTime("Time to predict result");
    }
    // Save the result
    std::string savepath = "C:\\Users\\zjc\\Desktop\\suspect_mask.png";
    cv::imwrite(savepath, mask);
    timer1.printElapsedTime("Time to save image");

    cv::waitKey(0);
    return 0;
}
