//
// Created by zjc on 24-11-12.
//

#ifndef TEMPLATE_MATCHING_H
#define TEMPLATE_MATCHING_H
void pre_process(const MIL_ID& inputImage, MIL_ID& outputImageSuspect, const std::map<std::string, int>& params);

void template_matching(const MIL_ID& inputImageSuspect, MIL_ID& outputImage, const std::map<std::string, int>& params);
#endif //TEMPLATE_MATCHING_H
