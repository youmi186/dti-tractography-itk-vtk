#ifndef TRACTOGRAPHY_LABELED_H
#define TRACTOGRAPHY_LABELED_H

#include <string>

void PerformTractographyLabeled(const std::string &tensorImagePath, const std::string &faImagePath, const std::string &labelImagePath, const std::string &eigenvectorImagePath, const std::string &outputImagePath);

#endif
