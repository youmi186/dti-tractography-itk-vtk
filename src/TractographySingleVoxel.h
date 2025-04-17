#ifndef TRACTOGRAPHY_SINGLE_VOXEL_H
#define TRACTOGRAPHY_SINGLE_VOXEL_H

#include <string>

void PerformTractographySingleVoxel(const std::string &tensorImagePath, const std::string &faImagePath, const std::string &eigenvectorImagePath, const std::string &outputImagePath);

#endif
