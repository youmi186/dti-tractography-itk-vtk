#ifndef SINGLE_SEED_FIBER_TRACK_H
#define SINGLE_SEED_FIBER_TRACK_H

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <array>
#include <vector>

class SingleSeedFiberTrack {
private:
    std::vector<float> vectorData;
    vtkSmartPointer<vtkImageData> faImage;
    std::vector<std::array<double, 3>> fiberPoints;
    double alpha;
    double stepSize;
    const int dimensions[3] = {144, 144, 85};
    const int MAX_STEPS = 200000;

    bool isInside(const std::array<double, 3>& point);
    double getFAValue(const std::array<double, 3>& point);
    std::array<double, 3> getVector(const std::array<double, 3>& point);

public:
    SingleSeedFiberTrack(const char* vectorBinFile, const char* faFile);
    void setParameters(double newAlpha, double newStepSize);
    void traceFiber(const std::array<double, 3>& seed);
    void visualize();
};

#endif // SINGLE_SEED_FIBER_TRACK_H