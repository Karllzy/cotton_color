//
// Created by zjc on 24-11-12.
//

#ifndef TEMPLATE_MATCHING_H
#define TEMPLATE_MATCHING_H


void pre_process(const MIL_ID& inputImage, MIL_ID& outputImageSuspect, const std::map<std::string, int>& params);
// void LoadTemplate(const MIL_ID &inputImage, MIL_ID &outputImage, std::map<std::string, int> &params);

extern std::vector<std::string> template_paths;
extern std::vector<MIL_INT> offsetX, offsetY, sizeX, sizeY;
extern std::vector<MIL_DOUBLE> drawColor;


class TemplateMatcher {
private:
    MIL_ID MilSystem;
    MIL_ID MilDisplay;
    MIL_ID MilSearchContext;
    MIL_ID MilResult;
    MIL_ID GraphicList;

    std::vector<std::string> ModelImgPaths;
    std::vector<MIL_INT> ModelsOffsetX;
    std::vector<MIL_INT> ModelsOffsetY;
    std::vector<MIL_INT> ModelsSizeX;
    std::vector<MIL_INT> ModelsSizeY;
    std::vector<MIL_DOUBLE> ModelsDrawColor;

    bool isInitialized;

    std::map<std::string, int> param;

public:
    // Constructor
    TemplateMatcher(MIL_ID& system, MIL_ID& display, std::map<std::string, int>& param);

    void LoadConfig(const std::string& config_path);

    // Load template models
    void loadTemplates(const std::vector<std::string>& template_paths,
                       const std::vector<MIL_INT>& offsetX, const std::vector<MIL_INT>& offsetY,
                       const std::vector<MIL_INT>& sizeX, const std::vector<MIL_INT>& sizeY,
                       const std::vector<MIL_DOUBLE>& drawColor);

    // Search for models in the input image
    void findModels(const MIL_ID& inputImage,MIL_ID& outputImage);



    void FindTemplates(const MIL_ID &inputImage, MIL_ID &outputImage,const std::map<std::string, int> &params);

    void predict(const MIL_ID& inputImage, MIL_ID& outputImage, const std::map<std::string, int> &params);

    static void loadConfig(const std::string& filename,
                std::vector<std::string>& template_paths,
                std::vector<MIL_INT>& offsetX,
                std::vector<MIL_INT>& offsetY,
                std::vector<MIL_INT>& sizeX,
                std::vector<MIL_INT>& sizeY,
                std::vector<MIL_DOUBLE>& drawColor);
    // Destructor
    ~TemplateMatcher();

};
// void FindTemplates( const MIL_ID& inputImage,const MIL_ID& outputImage,TemplateMatcher& matcher);
// void LoadTemplate(TemplateMatcher& matcher, std::map<std::string, int> &params);

#endif //TEMPLATE_MATCHING_H
