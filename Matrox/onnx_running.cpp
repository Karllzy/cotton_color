//
// Created by zjc on 24-11-13.
//

#include "onnx_running.h"


#include <mil.h>
//#include <milim.h> // 添加此行
#include <iostream>

/* Example functions declarations. */
void SingleModelExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/*****************************************************************************/
/* Main.
******************************************************************************/
int MosMain(void)
{
    MIL_ID MilApplication,     /* Application identifier. */
        MilSystem,          /* System Identifier.      */
        MilDisplay;         /* Display identifier.     */

    /* Allocate defaults. */
    MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

    /* Run single model example. */
    SingleModelExample(MilSystem, MilDisplay);

    /* Free defaults. */
    MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

    return 0;
}

/*****************************************************************************/
/* Single model example. */

/* Source MIL image file specifications. */
#define SINGLE_MODEL_IMAGE      MIL_TEXT("C:\\Users\\zjc\\Desktop\\diguandai2.png")  // 替换为您的模板RGB图像文件路径

/* Target MIL image file specifications. */
#define SINGLE_MODEL_TARGET_IMAGE  MIL_TEXT ("C:\\Users\\zjc\\Desktop\\diguandai.png")    // 替换为您的待检测RGB图像文件路径

/* Search speed: M_VERY_HIGH for faster search, M_MEDIUM for precision and robustness. */
#define SINGLE_MODEL_SEARCH_SPEED   M_LOW

/* Model specifications. */
#define MODEL_OFFSETX               3200L  // 根据您的模板图像调整
#define MODEL_OFFSETY               550L  // 根据您的模板图像调整
#define MODEL_SIZEX                200L  // 根据您的模板图像调整
#define MODEL_SIZEY                 200L  // 根据您的模板图像调整
#define MODEL_MAX_OCCURRENCES       6L

void SingleModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
{
    clock_t start_time = clock();

    MIL_ID      MilColorImage,                    /* 彩色图像缓冲区标识符。*/
        MilImage,                         /* 灰度图像缓冲区标识符。*/
        GraphicList;                      /* 图形列表标识符。*/
    MIL_ID      MilSearchContext,                 /* 搜索上下文。*/
        MilResult;                        /* 结果标识符。*/
    MIL_DOUBLE  ModelDrawColor = M_COLOR_RED;     /* 模板绘制颜色。*/
    MIL_INT     Model[MODEL_MAX_OCCURRENCES],     /* 模板索引。*/
        NumResults = 0L;                 /* 找到的结果数量。*/
    MIL_DOUBLE  Score[MODEL_MAX_OCCURRENCES],     /* 模板匹配得分。*/
        XPosition[MODEL_MAX_OCCURRENCES], /* 模板X位置。*/
        YPosition[MODEL_MAX_OCCURRENCES], /* 模板Y位置。*/
        Angle[MODEL_MAX_OCCURRENCES],     /* 模板角度。*/
        Scale[MODEL_MAX_OCCURRENCES],     /* 模板缩放。*/
        Time = 0.0;                       /* 计时变量。*/
    int         i;                                /* 循环变量。*/


    /* 加载RGB模板图像。 */
    MbufRestore(SINGLE_MODEL_IMAGE, MilSystem, &MilColorImage);

    /* 获取图像尺寸。 */
    MIL_INT Width = MbufInquire(MilColorImage, M_SIZE_X, M_NULL);
    MIL_INT Height = MbufInquire(MilColorImage, M_SIZE_Y, M_NULL);

    /* 分配灰度图像缓冲区。 */
    MbufAlloc2d(MilSystem, Width, Height, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilImage);

    /* 将RGB图像转换为灰度图像。 */
    MimConvert(MilColorImage, MilImage, M_RGB_TO_L);

    /* 选择灰度图像进行显示。 */
    MdispSelect(MilDisplay, MilImage);

    /* 释放彩色图像缓冲区。 */
    MbufFree(MilColorImage);

    /* Allocate a graphic list to hold the subpixel annotations to draw. */
    MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

    /* Associate the graphic list to the display for annotations. */
    MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

    /* Allocate a Geometric Model Finder context. */
    MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, &MilSearchContext);

    /* Allocate a result buffer. */
    MmodAllocResult(MilSystem, M_DEFAULT, &MilResult);

    /* Define the model. */
    MmodDefine(MilSearchContext, M_IMAGE, MilImage,
        MODEL_OFFSETX, MODEL_OFFSETY, MODEL_SIZEX, MODEL_SIZEY);

    /* Set the search speed. */
    MmodControl(MilSearchContext, M_CONTEXT, M_SPEED, SINGLE_MODEL_SEARCH_SPEED);

    /* Preprocess the search context. */
    MmodPreprocess(MilSearchContext, M_DEFAULT);

    /* Draw box and position it in the source image to show the model. */
    MgraColor(M_DEFAULT, ModelDrawColor);
    MmodDraw(M_DEFAULT, MilSearchContext, GraphicList,
        M_DRAW_BOX + M_DRAW_POSITION, 0, M_ORIGINAL);
    clock_t end_time = clock();
    std::cout << "The run time is: " << (double)(end_time - start_time) / CLOCKS_PER_SEC << "s";
    /* Pause to show the model. */
    MosPrintf(MIL_TEXT("\nGEOMETRIC MODEL FINDER:\n"));
    MosPrintf(MIL_TEXT("-----------------------\n\n"));
    MosPrintf(MIL_TEXT("A model context was defined with "));
    MosPrintf(MIL_TEXT("the model in the displayed image.\n"));
    MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
    MosGetch();

    /* Clear annotations. */
    MgraClear(M_DEFAULT, GraphicList);

    /* 加载RGB待检测图像。 */
    MbufRestore(SINGLE_MODEL_TARGET_IMAGE, MilSystem, &MilColorImage);

    /* 确保待检测图像的尺寸与模板图像一致。 */
    MIL_INT TargetWidth = MbufInquire(MilColorImage, M_SIZE_X, M_NULL);
    MIL_INT TargetHeight = MbufInquire(MilColorImage, M_SIZE_Y, M_NULL);

    /* 如果尺寸不同，需要重新分配灰度图像缓冲区。 */
    if (TargetWidth != Width || TargetHeight != Height)
    {
        /* 释放之前的灰度图像缓冲区。 */
        MbufFree(MilImage);

        /* 分配新的灰度图像缓冲区。 */
        MbufAlloc2d(MilSystem, TargetWidth, TargetHeight, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilImage);

        /* 更新宽度和高度。 */
        Width = TargetWidth;
        Height = TargetHeight;
    }

    /* 将RGB待检测图像转换为灰度图像。 */
    MimConvert(MilColorImage, MilImage, M_RGB_TO_L);

    /* 释放彩色图像缓冲区。 */
    MbufFree(MilColorImage);

    /* 显示灰度待检测图像。 */
    MdispSelect(MilDisplay, MilImage);

    /* Dummy first call for bench measure purpose only (bench stabilization,
       cache effect, etc...). This first call is NOT required by the application. */
    MmodFind(MilSearchContext, MilImage, MilResult);

    /* Reset the timer. */
    MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

    /* Find the model. */
    MmodFind(MilSearchContext, MilImage, MilResult);

    /* Read the find time. */
    MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

    /* Get the number of models found. */
    MmodGetResult(MilResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

    /* If a model was found above the acceptance threshold. */
    if ((NumResults >= 1) && (NumResults <= MODEL_MAX_OCCURRENCES))
    {
        /* Get the results of the single model. */
        MmodGetResult(MilResult, M_DEFAULT, M_INDEX + M_TYPE_MIL_INT, Model);
        MmodGetResult(MilResult, M_DEFAULT, M_POSITION_X, XPosition);
        MmodGetResult(MilResult, M_DEFAULT, M_POSITION_Y, YPosition);
        MmodGetResult(MilResult, M_DEFAULT, M_ANGLE, Angle);
        MmodGetResult(MilResult, M_DEFAULT, M_SCALE, Scale);
        MmodGetResult(MilResult, M_DEFAULT, M_SCORE, Score);

        /* Print the results for each model found. */
        MosPrintf(MIL_TEXT("The model was found in the target image:\n\n"));
        MosPrintf(MIL_TEXT("Result   Model   X Position   Y Position   ")
            MIL_TEXT("Angle   Scale   Score\n\n"));
        for (i = 0; i < NumResults; i++)
        {
            MosPrintf(MIL_TEXT("%-9d%-8d%-13.2f%-13.2f%-8.2f%-8.2f%-5.2f%%\n"),
                i, (int)Model[i], XPosition[i], YPosition[i],
                Angle[i], Scale[i], Score[i]);
        }
        MosPrintf(MIL_TEXT("\nThe search time is %.1f ms\n\n"), Time * 1000.0);

        /* Draw edges, position and box over the occurrences that were found. */
        for (i = 0; i < NumResults; i++)
        {
            MgraColor(M_DEFAULT, ModelDrawColor);
            MmodDraw(M_DEFAULT, MilResult, GraphicList,
                M_DRAW_EDGES + M_DRAW_BOX + M_DRAW_POSITION, i, M_DEFAULT);
        }
    }
    else
    {
        MosPrintf(MIL_TEXT("The model was not found or the number of models ")
            MIL_TEXT("found is greater than\n"));
        MosPrintf(MIL_TEXT("the specified maximum number of occurrence !\n\n"));
    }

    /* Wait for a key to be pressed. */
    MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
    MosGetch();

    /* Free MIL objects. */
    MgraFree(GraphicList);
    MbufFree(MilImage);
    MmodFree(MilSearchContext);
    MmodFree(MilResult);
}
