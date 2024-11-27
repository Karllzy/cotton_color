#include <execution>
#include <mil.h>
#include <iostream>
#include "color_range.h"
#include "utils.h"
#include"template_matching.h"
using namespace std;

void pre_process(const MIL_ID& inputImage, MIL_ID& outputImageSuspect, const map<string, int>& params) {
    // 异色检测, 检测出不同与棉花颜色的物体作为模板匹配的对象
    const vector<string> colors = {"cotton", "background"};
    map <string, int> param_temp = params;
    param_temp["lab_denoising"] = param_temp["cotton_denoising"];
    param_temp["saturation_threshold"] = param_temp["cotton_saturation_threshold"];
    param_temp["saturation_denoising"] = param_temp["cotton_saturation_denoising"];
    lab_process_raw(inputImage, outputImageSuspect, param_temp, colors);
    MimArith(outputImageSuspect, M_NULL, outputImageSuspect, M_NOT);
}

TemplateMatcher::TemplateMatcher(MIL_ID system, MIL_ID display, std::map<std::string, int>& param)
    : MilSystem(system), MilDisplay(display), isInitialized(false), param(param)
{
}

// Destructor
TemplateMatcher::~TemplateMatcher()
{
    if (isInitialized) {
        MgraFree(GraphicList);
        MmodFree(MilSearchContext);
        MmodFree(MilResult);
    }
}

// Load template models
void TemplateMatcher::loadTemplates(const std::vector<std::string>& template_paths,
                                    const std::vector<MIL_INT>& offsetX,
                                    const std::vector<MIL_INT>& offsetY,
                                    const std::vector<MIL_INT>& sizeX,
                                    const std::vector<MIL_INT>& sizeY,
                                    const std::vector<MIL_DOUBLE>& drawColor)
{
    if (isInitialized) {
        std::cerr << "Templates are already loaded. Skipping reloading.\n";
        return;
    }

    ModelsOffsetX = offsetX;
    ModelsOffsetY = offsetY;
    ModelsSizeX = sizeX;
    ModelsSizeY = sizeY;
    ModelsDrawColor = drawColor;
    ModelImgPaths = template_paths;

    // Allocate search context and result buffers
    MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, &MilSearchContext);
    MmodAllocResult(MilSystem, M_DEFAULT, &MilResult);

    // Allocate a graphic list to hold the annotations
    MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);
    MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

    // Define templates
    for (size_t i = 0; i < template_paths.size(); ++i) {
        MIL_ID template_temporary;
        MgraClear(M_DEFAULT, GraphicList);
        MbufRestore(convert_to_wstring(ModelImgPaths[i]).c_str(), MilSystem, &template_temporary);
        MIL_ID template_temporary_uint8 = convert_to_uint8(template_temporary);
        if (this->param["isdisplay"] == 1)
        {
            MdispSelect(MilDisplay, template_temporary_uint8);
        }

        MmodDefine(MilSearchContext, M_IMAGE, template_temporary_uint8,
                   static_cast<MIL_DOUBLE>(ModelsOffsetX[i]),
                   static_cast<MIL_DOUBLE>(ModelsOffsetY[i]),
                   static_cast<MIL_DOUBLE>(ModelsSizeX[i]),
                   static_cast<MIL_DOUBLE>(ModelsSizeY[i]));

        MgraColor(M_DEFAULT, ModelsDrawColor[i]);
        MmodDraw(M_DEFAULT, MilSearchContext, GraphicList,
                 M_DRAW_BOX + M_DRAW_POSITION, i, M_ORIGINAL);

        if (this->param["isdisplay"] == 1)
        {
            MosGetch();
        }
        MbufFree(template_temporary);
        MbufFree(template_temporary_uint8);
    }

    // Set parameters
    MmodControl(MilSearchContext, M_CONTEXT, M_SPEED, M_VERY_HIGH);
    MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, 75);
    MmodControl(MilSearchContext, M_DEFAULT, M_ACCEPTANCE, 40);
    MmodControl(MilSearchContext, M_DEFAULT, M_CERTAINTY,  60);
    MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, 2);

    // Preprocess templates
    MmodPreprocess(MilSearchContext, M_DEFAULT);

    isInitialized = true;
    std::cout << "Templates loaded and preprocessed successfully.\n";

    // Pause to show the models
    MosPrintf(MIL_TEXT("A model context was defined with the ")
              MIL_TEXT("models in the displayed image.\n"));
    MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
    if (this->param["debug_mode"] == 1) {
        MosGetch();
    }
}

// Search for models in the input image
void TemplateMatcher::findModels(const MIL_ID& inputImage,MIL_ID& outputImage)
{
    if (!isInitialized) {
        std::cerr << "Templates are not loaded. Please load templates before searching.\n";
        return;
    }
    MIL_ID input_image_uint8 = convert_to_uint8(inputImage);
    if(this->param["isdisplay"] == 1) {
        MdispSelect(MilDisplay, input_image_uint8);
    }

    // Clear previous annotations
    MgraClear(M_DEFAULT, GraphicList);

    // Find models
    MIL_DOUBLE Time = 0.0;
    MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
    MmodFind(MilSearchContext, input_image_uint8, MilResult);
    MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

    // Get results
    MIL_INT NumResults = 0;
    MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

    if (NumResults >= 1) {
        std::vector<MIL_INT> Models(NumResults);
        std::vector<MIL_DOUBLE> XPosition(NumResults), YPosition(NumResults), Angle(NumResults),
            Scale(NumResults), Score(NumResults);

        MmodGetResult(MilResult, M_DEFAULT, M_INDEX + M_TYPE_MIL_INT, Models.data());
        MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition.data());
        MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition.data());
        MmodGetResult(MilResult, M_DEFAULT, M_ANGLE, Angle.data());
        MmodGetResult(MilResult, M_DEFAULT, M_SCALE, Scale.data());
        MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score.data());

        // Create a binary image buffer
        MbufAlloc2d(MilSystem, MbufInquire(inputImage, M_SIZE_X, M_NULL),
              MbufInquire(inputImage, M_SIZE_Y, M_NULL), 1 + M_UNSIGNED,
              M_IMAGE + M_PROC, &outputImage);
        // Initialize the binary image to black
        MbufClear(outputImage, 0);

        // Display results
        if(this->param["debug_mode"] == 1) {
            std::cout << "Found " << NumResults << " model(s) in " << Time * 1000.0 << " ms:\n";
            std::cout << "Result   Model   X Position   Y Position   Angle   Scale   Score\n";
        }
        for (MIL_INT i = 0; i < NumResults; ++i) {
            if (this->param["debug_mode"] == 1) {
                std::cout << i << "        " << Models[i] << "        " << XPosition[i] << "        "
                          << YPosition[i] << "        " << Angle[i] << "        " << Scale[i]
                          << "        " << Score[i] << "%\n";
            }
            // Draw results onto the binary image
            MgraColor(M_DEFAULT, 255); // White color for binary image
            MmodDraw(M_DEFAULT, MilResult, outputImage, M_DRAW_EDGES + M_DRAW_POSITION, i, M_DEFAULT);

            // Draw results on the graphical list for display
            MgraColor(M_DEFAULT, ModelsDrawColor[Models[i]]);
            MmodDraw(M_DEFAULT, MilResult, GraphicList,
                     M_DRAW_EDGES + M_DRAW_POSITION, i, M_DEFAULT);
        }

    } else {
        std::cout << "No models found.\n";
    }
    // MosPrintf(MIL_TEXT("Press <Enter> to EXIT.\n\n"));
    // MosGetch();
    MbufFree(input_image_uint8);
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

void TemplateMatcher::loadConfig(const std::string& filename,
                std::vector<std::string>& template_paths,
                std::vector<MIL_INT>& offsetX,
                std::vector<MIL_INT>& offsetY,
                std::vector<MIL_INT>& sizeX,
                std::vector<MIL_INT>& sizeY,
                std::vector<MIL_DOUBLE>& drawColor) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open configuration file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        auto values = splitString(value, ',');

        if (key == "template_paths") {
            template_paths = values;
        } else if (key == "offsetX") {
            for (const auto& v : values) offsetX.push_back(std::stoi(v));
        } else if (key == "offsetY") {
            for (const auto& v : values) offsetY.push_back(std::stoi(v));
        } else if (key == "sizeX") {
            for (const auto& v : values) sizeX.push_back(std::stoi(v));
        } else if (key == "sizeY") {
            for (const auto& v : values) sizeY.push_back(std::stoi(v));
        } else if (key == "drawColor") {
            for (const auto& v : values) {
                if (v == "M_COLOR_RED") drawColor.push_back(M_COLOR_RED);
                else if (v == "M_COLOR_GREEN") drawColor.push_back(M_COLOR_GREEN);
                else if (v == "M_COLOR_BLUE") drawColor.push_back(M_COLOR_BLUE);
            }
        }
    }
    file.close();
}

void TemplateMatcher::LoadConfig(const string& config_path)
{
    std::vector<std::string> template_paths;
    std::vector<MIL_INT> offsetX, offsetY, sizeX, sizeY;
    std::vector<MIL_DOUBLE> drawColor;

    // 调用 loadConfig 并加载配置
    loadConfig(config_path, template_paths, offsetX, offsetY, sizeX, sizeY, drawColor);

    // 调用 matcher 的 loadTemplates 方法
    this->loadTemplates(template_paths, offsetX, offsetY, sizeX, sizeY, drawColor);
}


void TemplateMatcher::FindTemplates( const MIL_ID& inputImage, MIL_ID& outputImage,const std::map<std::string, int> &params)
{
    // Perform template matching
    this -> findModels(inputImage,outputImage);
}

void TemplateMatcher::predict(const MIL_ID &inputImage, MIL_ID &outputImage, const std::map<std::string, int> &params) {
    MIL_ID detection_result, detection_resize, output_image_unresize;
    pre_process(inputImage, detection_result, params);
    MbufAlloc2d(MilSystem, MbufInquire(detection_result, M_SIZE_X, M_NULL)/2,
       MbufInquire(detection_result, M_SIZE_Y, M_NULL)/2, 1 + M_UNSIGNED,
       M_IMAGE + M_PROC, &detection_resize);
    MimResize(detection_result,detection_resize,0.5,0.5,M_DEFAULT);
    this->FindTemplates(detection_resize,output_image_unresize, params);
    MbufAlloc2d(MilSystem, MbufInquire(detection_result, M_SIZE_X, M_NULL),
       MbufInquire(detection_result, M_SIZE_Y, M_NULL), 1 + M_UNSIGNED,
       M_IMAGE + M_PROC, &outputImage);
    MimResize(output_image_unresize,outputImage,2,2,M_DEFAULT);
    MbufFree(detection_result);
    MbufFree(detection_resize);
    MbufFree(output_image_unresize);
}


// TODO: Opencv ONNX runner,    已
//       1. 构建相应的模型加载和模型运行函数-
//       2. 在src里头添加另一个cvdl库，专用于视觉深度学习
//       3. 添加一个类OnnxRunner

// TODO: 完善config文件，确保能够读取mask转换的相关参数  已

// TODO: Opencv和matrox图像的转换函数，添加到Matrox/utils.cpp   有问题

// TODO：构建统一的图像检测器类，可以一键加载，一键开启多进程快速预测

// TODO：计算统一预测框架的预测时间

// TODO: 完善模板和参数，添加陈棉模块，陈棉模块可通过配置进行启用和关闭。

//TODO: 完善相应部分的手册 已

