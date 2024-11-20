#include <execution>
#include <mil.h>
#include <iostream>
#include "color_range.h"
#include "utils.h"
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


class TemplateMatcher {
    private:
        MIL_ID MilSearchContext;
        MIL_ID MilResult;
        MIL_ID GraphicList;

        vector<string> ModelImgPaths;
        vector<MIL_INT> ModelsOffsetX;
        vector<MIL_INT> ModelsOffsetY;
        vector<MIL_INT> ModelsSizeX;
        vector<MIL_INT> ModelsSizeY;
        vector<MIL_DOUBLE> ModelsDrawColor;

        bool isInitialized;

    public:
    // Constructor
    TemplateMatcher(MIL_ID system, MIL_ID display):
            isInitialized(false) {}

    // Destructor: Free MIL objects
    ~TemplateMatcher() {
        if (isInitialized) {
            MgraFree(GraphicList);
            MmodFree(MilSearchContext);
            MmodFree(MilResult);
        }
    }

    // Load template models
    void loadTemplates(const vector<string>& template_paths,
                       const vector<MIL_INT>& offsetX, const vector<MIL_INT>& offsetY,
                       const vector<MIL_INT>& sizeX, const vector<MIL_INT>& sizeY,
                       const vector<MIL_DOUBLE>& drawColor) {

        if (isInitialized) {
            cerr << "Templates are already loaded. Reinitializing...\n";
            MgraFree(GraphicList);
            MmodFree(MilSearchContext);
            MmodFree(MilResult);
        }

        ModelsOffsetX = offsetX;
        ModelsOffsetY = offsetY;
        ModelsSizeX = sizeX;
        ModelsSizeY = sizeY;
        ModelsDrawColor = drawColor;
        ModelImgPaths = template_paths;

        // 搜索上下文和搜索结果的buffer
        MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, &MilSearchContext);
        MmodAllocResult(MilSystem, M_DEFAULT, &MilResult);

        // Define templates
        for (size_t i = 0; i < template_paths.size(); ++i) {
            MIL_ID template_temporary;
            MbufRestore(convert_to_wstring(ModelImgPaths[i]), MilSystem, &template_temporary);
            MIL_ID template_temporary_uint8 = convert_to_uint8(template_temporary);
            MdispSelect(MilDisplay, template_temporary_uint8);
            /* Allocate a graphic list to hold the subpixel annotations to draw. */
            MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);
            /* Associate the graphic list to the display for annotations. */
            MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);



            MmodDefine(MilSearchContext, M_IMAGE, template_temporary_uint8,
                       static_cast<MIL_DOUBLE>(ModelsOffsetX[i]),
                       static_cast<MIL_DOUBLE>(ModelsOffsetY[i]),
                       static_cast<MIL_DOUBLE>(ModelsSizeX[i]),
                       static_cast<MIL_DOUBLE>(ModelsSizeY[i]));
        }

        // 设置一些参数
        /* Set the desired search speed. */
        MmodControl(MilSearchContext, M_CONTEXT, M_SPEED, M_VERY_HIGH);

        /* Increase the smoothness for the edge extraction in the search context. */
        MmodControl(MilSearchContext, M_CONTEXT, M_SMOOTHNESS, 75);

        /* Modify the acceptance and the certainty for all the models that were defined. */
        MmodControl(MilSearchContext, M_DEFAULT, M_ACCEPTANCE, 40);
        MmodControl(MilSearchContext, M_DEFAULT, M_CERTAINTY,  60);

        /* Set the number of occurrences to 2 for all the models that were defined. */
        MmodControl(MilSearchContext, M_DEFAULT, M_NUMBER, 2);

        // Preprocess templates
        MmodPreprocess(MilSearchContext, M_DEFAULT);

        isInitialized = true;
        cout << "Templates loaded and preprocessed successfully.\n";

        /* Draw boxes and positions in the source image to identify the models. */
        for (int i=0; i<ModelImgPaths.size(); i++)
        {
            MgraColor(M_DEFAULT, ModelsDrawColor[i]);
            MmodDraw( M_DEFAULT, MilSearchContext, GraphicList,
                      M_DRAW_BOX+M_DRAW_POSITION, i, M_ORIGINAL);
        }

        /* Pause to show the models. */
        MosPrintf(MIL_TEXT("A model context was defined with the ")
                       MIL_TEXT("models in the displayed image.\n"));
        MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
        MosGetch();

    }

    // Search for models in the input image
    void findModels(const MIL_ID& inputImage) {
        if (!isInitialized) {
            cerr << "Templates are not loaded. Please load templates before searching.\n";
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
            vector<MIL_INT> Models(NumResults);
            vector<MIL_DOUBLE> XPosition(NumResults), YPosition(NumResults), Angle(NumResults),
                Scale(NumResults), Score(NumResults);

            MmodGetResult(MilResult, M_DEFAULT, M_INDEX + M_TYPE_MIL_INT, Models.data());
            MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition.data());
            MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition.data());
            MmodGetResult(MilResult, M_DEFAULT, M_ANGLE, Angle.data());
            MmodGetResult(MilResult, M_DEFAULT, M_SCALE, Scale.data());
            MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score.data());

            // Display results
            cout << "Found " << NumResults << " model(s) in " << Time * 1000.0 << " ms:\n";
            cout << "Result   Model   X Position   Y Position   Angle   Scale   Score\n";
            for (MIL_INT i = 0; i < NumResults; ++i) {
                cout << i << "        " << Models[i] << "        " << XPosition[i] << "        "
                          << YPosition[i] << "        " << Angle[i] << "        " << Scale[i]
                          << "        " << Score[i] << "%\n";

                // Draw results
                MgraColor(M_DEFAULT, ModelsDrawColor[Models[i]]);
                MmodDraw(M_DEFAULT, MilResult, GraphicList,
                         M_DRAW_EDGES + M_DRAW_POSITION, i, M_DEFAULT);
            }
        } else {
            cout << "No models found.\n";
        }
    }
};


void template_matching(const MIL_ID& inputImage, MIL_ID& outputImage, const map<string, int>& params) {
    // Create a TemplateMatcher instance
    TemplateMatcher matcher(MilSystem, MilDisplay);

    // Load template models
    vector<string>  template_paths = {"C:\\Users\\zjc\\Desktop\\template1.png",
        "C:\\Users\\zjc\\Desktop\\template1.png",
        "C:\\Users\\zjc\\Desktop\\template1.png",
    };
    vector<MIL_INT> offsetX = {0, 20, 30};
    vector<MIL_INT> offsetY = {0, 20, 30};
    vector<MIL_INT> sizeX = {100, 60, 40};
    vector<MIL_INT> sizeY = {100, 60, 40};
    vector<MIL_DOUBLE> drawColor = {M_COLOR_RED, M_COLOR_GREEN, M_COLOR_BLUE};
    matcher.loadTemplates(template_paths, offsetX, offsetY, sizeX, sizeY, drawColor);

    // Find models
    matcher.findModels(inputImage);

    // Free resources
    MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, MilDisplay);
}

