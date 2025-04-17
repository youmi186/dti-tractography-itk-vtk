#ifndef FREE_FIBER_TRACK_H
#define FREE_FIBER_TRACK_H

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <array>
#include <vector>

// 全局变量声明
extern std::array<double, 3> SeedPoint;
extern bool SeedPointUpdated;

// 纤维追踪数据结构
struct FiberTrack {
    std::vector<std::array<double, 3>> points;
    std::array<double, 3> color;
};

// 自定义交互器类
class CustomInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    static CustomInteractorStyle* New();
    vtkTypeMacro(CustomInteractorStyle, vtkInteractorStyleTrackballCamera);
    virtual void OnLeftButtonDown() override;
};

// 主要的纤维追踪类
class FreeFiberTrack {
private:
    std::vector<float> vectorData;
    vtkSmartPointer<vtkImageData> faImage;
    std::vector<FiberTrack> fiberTracks;
    double alpha;
    double stepSize;
    const int dimensions[3] = {144, 144, 85};
    const int MAX_STEPS = 200000;

    std::array<double, 3> generateColor(int trackIndex);
    bool isInside(const std::array<double, 3>& point);
    double getFAValue(const std::array<double, 3>& point);
    std::array<double, 3> getVector(const std::array<double, 3>& point);

public:
    FreeFiberTrack(const char* vectorBinFile, const char* faFile);
    void setParameters(double newAlpha, double newStepSize);
    void traceFiber(const std::array<double, 3>& seed);
    void visualize();
};

#endif // FREE_FIBER_TRACK_H