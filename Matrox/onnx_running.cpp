//
// Created by zjc on 24-11-13.
//

#include "onnx_running.h"


#include <mil.h>
//#include <milim.h> // 添加此行
#include <iostream>
#include <mil.h>

// Path definitions.
#define EXAMPLE_ONNX_MODEL_PATH  MIL_TEXT("C:\\Users\\zjc\\Desktop\\WeChat Files\\wxid_ipl8u0ctajtn22\\FileStorage\\File\\2024-11\\best(2).onnx")
#define TARGET_IMAGE_DIR_PATH    MIL_TEXT("C:\\Users\\zjc\\Desktop\\dimo2.bmp")

int MosMain(void)
{
    MIL_ID MilApplication = M_NULL,   // MIL application identifier
        MilSystem = M_NULL,            // MIL system identifier
        MilDisplay = M_NULL,           // MIL display identifier
        MilImage = M_NULL,             // MIL image identifier
        MilDetectedImage = M_NULL,     // MIL image with detections
        DetectCtx = M_NULL,            // MIL ONNX detection context
        DetectRes = M_NULL;            // MIL detection result

    // Allocate MIL objects.
    MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
    MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSystem);
    MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);

    // Load the image into memory.
    if (MbufRestore(TARGET_IMAGE_DIR_PATH, MilSystem, &MilImage) != M_NULL)
    {
        MosPrintf(MIL_TEXT("Image loaded successfully.\n"));
    }
    else
    {
        MosPrintf(MIL_TEXT("Failed to load image.\n"));
        return 1;  // Exit if the image loading failed
    }

    // Import the YOLOv5 ONNX model into the detection context.
    MosPrintf(MIL_TEXT("Importing the YOLOv5 ONNX model into the detection context...\n"));
    MclassAlloc(MilSystem, M_CLASSIFIER_ONNX, M_DEFAULT, &DetectCtx);
    MclassImport(EXAMPLE_ONNX_MODEL_PATH, M_ONNX_FILE, DetectCtx, M_DEFAULT, M_DEFAULT, M_DEFAULT);
    MosPrintf(MIL_TEXT("Model imported successfully.\n"));

    // Preprocess the detection context.
    MclassPreprocess(DetectCtx, M_DEFAULT);  // Ensure the context is preprocessed.

    // Allocate a detection result buffer.
    MclassAllocResult(MilSystem, M_PREDICT_ONNX_RESULT, M_DEFAULT, &DetectRes);

    // Perform object detection on the image using MclassPredict.
    MclassPredict(DetectCtx, MilImage, DetectRes, M_DEFAULT);
    MosPrintf(MIL_TEXT("Object detection completed.\n"));

    // Allocate a buffer for displaying the detection results.
    MbufAlloc2d(MilSystem, 640, 640, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilDetectedImage);

    // Retrieve and draw the detection results manually.
    MIL_INT NumDetections = 0;
    MclassGetResult(DetectRes, M_GENERAL, M_TYPE_MIL_INT, &NumDetections);

    if (NumDetections > 0)
    {
        for (MIL_INT i = 0; i < NumDetections; i++)
        {
            MIL_DOUBLE Score;
            MIL_INT ClassIndex;
            MIL_DOUBLE BBoxX, BBoxY, BBoxWidth, BBoxHeight;

            // Retrieve detection results for each object.
            MclassGetResult(DetectRes, i, M_SCORE + M_TYPE_MIL_DOUBLE, &Score);
            MclassGetResult(DetectRes, i, M_INDEX + M_TYPE_MIL_INT, &ClassIndex);
            MclassGetResult(DetectRes, i, M_SEED_VALUE + M_TYPE_MIL_DOUBLE, &BBoxX);
            MclassGetResult(DetectRes, i, M_SEED_VALUE + M_TYPE_MIL_DOUBLE, &BBoxY);
            MclassGetResult(DetectRes, i, M_SEED_VALUE + M_TYPE_MIL_DOUBLE, &BBoxWidth);
            MclassGetResult(DetectRes, i, M_SEED_VALUE + M_TYPE_MIL_DOUBLE, &BBoxHeight);

            // Draw bounding box.
            MgraColor(M_DEFAULT, M_COLOR_GREEN);
            MgraRect(M_DEFAULT, MilDetectedImage, BBoxX, BBoxY, BBoxX + BBoxWidth, BBoxY + BBoxHeight);

            // Optionally, display detection score or class name (if needed).
            MIL_TEXT_CHAR Label[256];
            MosSprintf(Label, 256, MIL_TEXT("Class %d: %.2lf%%"), ClassIndex, Score * 100);
            MgraFont(M_DEFAULT, M_FONT_DEFAULT_SMALL);
            MgraText(M_DEFAULT, MilDetectedImage, BBoxX, BBoxY - 10, Label);
        }
    }
    else
    {
        MosPrintf(MIL_TEXT("No detections found.\n"));
    }

    // Display the image with detection results.
    MdispSelect(MilDisplay, MilDetectedImage);

    // Wait for the user to close the window.
    MosPrintf(MIL_TEXT("Press <Enter> to exit.\n"));
    MosGetch();

    // Free all allocated resources.
    MbufFree(MilImage);
    MbufFree(MilDetectedImage);
    MclassFree(DetectRes);
    MclassFree(DetectCtx);
    MdispFree(MilDisplay);
    MsysFree(MilSystem);
    MappFree(MilApplication);

    return 0;
}

