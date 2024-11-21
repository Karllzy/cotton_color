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
void TemplateMatcher::findModels(const MIL_ID& inputImage)
{
    if (!isInitialized) {
        std::cerr << "Templates are not loaded. Please load templates before searching.\n";
        return;
    }
    MIL_ID input_image_uint8 = convert_to_uint8(inputImage);

    MdispSelect(MilDisplay, input_image_uint8);

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

        // Display results
        std::cout << "Found " << NumResults << " model(s) in " << Time * 1000.0 << " ms:\n";
        std::cout << "Result   Model   X Position   Y Position   Angle   Scale   Score\n";
        for (MIL_INT i = 0; i < NumResults; ++i) {
            std::cout << i << "        " << Models[i] << "        " << XPosition[i] << "        "
                      << YPosition[i] << "        " << Angle[i] << "        " << Scale[i]
                      << "        " << Score[i] << "%\n";

            // Draw results
            MgraColor(M_DEFAULT, ModelsDrawColor[Models[i]]);
            MmodDraw(M_DEFAULT, MilResult, GraphicList,
                     M_DRAW_EDGES + M_DRAW_POSITION, i, M_DEFAULT);
        }
    } else {
        std::cout << "No models found.\n";
    }
    MosPrintf(MIL_TEXT("Press <Enter> to EXIT.\n\n"));
    MosGetch();
    MbufFree(input_image_uint8);
}
void TemplateMatcher::LoadTemplate(TemplateMatcher& matcher, std::map<std::string, int> &params)
{
    // Create a TemplateMatcher instance (consider making it static if you want to retain it between calls)

    // Load template models only once
    matcher.loadTemplates(
        {
            "C:\\Users\\zjc\\Desktop\\templates\\template1.png",
            "C:\\Users\\zjc\\Desktop\\templates\\template2.png",
            "C:\\Users\\zjc\\Desktop\\templates\\template3.png",
            "C:\\Users\\zjc\\Desktop\\templates\\template4.png",
            "C:\\Users\\zjc\\Desktop\\templates\\template5.png",
            "C:\\Users\\zjc\\Desktop\\templates\\template6.png",
        },
        {0, 20, 30, 10,10,10},  // offsetX
        {0, 20, 30, 10,10,10},  // offsetY
        {100, 60, 60, 66,66,66}, // sizeX
        {100, 60, 60, 66,66,66}, // sizeY
        {M_COLOR_RED, M_COLOR_GREEN, M_COLOR_BLUE, M_COLOR_GREEN,M_COLOR_BLUE,M_COLOR_BLUE} // drawColor
    );
}

void TemplateMatcher::FindTemplates( const MIL_ID& inputImage,const MIL_ID& outputImage,TemplateMatcher& matcher)
{
    // Perform template matching
    matcher.findModels(inputImage);

    // Notify user that matching is complete
    cout << "Template matching completed.\n";
}

//TODO: 1加入加载多个模板的功能   已
//TODO: 2加入配置文件解析功能，解析后的文件与当前的para map<string, int>兼容
//       配置文件当中加入是否显示参数，能调控加载模板的过程是否显示。已
//TODO: 3修改当前的代码使模板匹配不出错  已
//TODO: 4成立模板文件夹，能够加载文件夹下的全部模板并实现检测 已
//TODO: 5制作标准结构的函数，例如：matcher.findModels(MIL_ID inputImage, MIL_ID output_image, map);
////未实现，因为加载和寻找分开后，要对加载和寻找函数传入类成员，无法统一，其余可用到的参数统一，加一个类成员即可。
//TODO: 6完善相应部分的手册 已

