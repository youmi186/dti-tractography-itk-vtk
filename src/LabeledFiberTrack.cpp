#include "LabeledFiberTrack.h"
#include <vtkNrrdReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLine.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <queue>
#include <fstream>
#include <unistd.h>

LabeledFiberTrack::LabeledFiberTrack(const char* vectorBinFile, const char* faFile)
    : alpha(0.5), stepSize(1.0) {
    std::ifstream binFile(vectorBinFile, std::ios::binary);
    size_t totalElements = dimensions[0] * dimensions[1] * dimensions[2] * 3;
    vectorData.resize(totalElements);
    binFile.read(reinterpret_cast<char*>(vectorData.data()), totalElements * sizeof(float));
    binFile.close();

    auto faReader = vtkSmartPointer<vtkNrrdReader>::New();
    faReader->SetFileName(faFile);
    faReader->Update();
    faImage = faReader->GetOutput();
}

void LabeledFiberTrack::setParameters(double newAlpha, double newStepSize) {
    alpha = newAlpha;
    stepSize = newStepSize;
}

bool LabeledFiberTrack::isInside(const std::array<double, 3>& point) {
    return point[0] >= 0 && point[0] < dimensions[0] &&
           point[1] >= 0 && point[1] < dimensions[1] &&
           point[2] >= 0 && point[2] < dimensions[2];
}

double LabeledFiberTrack::getFAValue(const std::array<double, 3>& point) {
    return faImage->GetScalarComponentAsDouble(
        static_cast<int>(point[0]),
        static_cast<int>(point[1]),
        static_cast<int>(point[2]),
        0
    );
}

std::array<double, 3> LabeledFiberTrack::getVector(const std::array<double, 3>& point) {
    int x = static_cast<int>(point[0]);
    int y = static_cast<int>(point[1]);
    int z = static_cast<int>(point[2]);

    size_t baseIdx = (x * dimensions[1] * dimensions[2] + y * dimensions[2] + z) * 3;
    std::array<double, 3> vec;
    for(int i = 0; i < 3; i++) {
        vec[i] = vectorData[baseIdx + i];
    }
    return vec;
}

void LabeledFiberTrack::traceFiber(const std::array<double, 3>& seed) {
    std::queue<std::array<double, 3>> q;
    q.push(seed);
    int stepCount = 0;

    while (!q.empty() && stepCount < MAX_STEPS) {
        auto currentPoint = q.front();
        q.pop();
        fiberPoints.push_back(currentPoint);
        auto vec = getVector(currentPoint);

        for (int direction : {-1, 1}) {
            std::array<double, 3> nextPoint;
            for (int i = 0; i < 3; i++) {
                nextPoint[i] = currentPoint[i] + stepSize * vec[i] * direction;
            }

            if (!isInside(nextPoint)) {
                continue;
            }

            double nextFA = getFAValue(nextPoint);
            if (nextFA < alpha) {
                continue;
            }

            q.push(nextPoint);
        }
        visualize();
        stepCount++;
    }
}

std::vector<std::array<double, 3>> LabeledFiberTrack::findSeedPoints(const char* labelFile) {
    std::vector<std::array<double, 3>> seedPoints;

    auto labelReader = vtkSmartPointer<vtkNrrdReader>::New();
    labelReader->SetFileName(labelFile);
    labelReader->Update();
    auto labelImage = labelReader->GetOutput();

    for(int x = 0; x < dimensions[0]; x++) {
        for(int y = 0; y < dimensions[1]; y++) {
            for(int z = 0; z < dimensions[2]; z++) {
                if(labelImage->GetScalarComponentAsDouble(x, y, z, 0) == 1.0) {
                    seedPoints.push_back({static_cast<double>(x),
                                       static_cast<double>(y),
                                       static_cast<double>(z)});
                }
            }
        }
    }

    return seedPoints;
}

void LabeledFiberTrack::traceAllFibers(const char* labelFile) {
    fiberPoints.clear();
    auto seedPoints = findSeedPoints(labelFile);
    for(const auto& seed : seedPoints) {
        traceFiber(seed);
    }
}

void LabeledFiberTrack::visualize() {
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    for(size_t i = 0; i < fiberPoints.size(); i++) {
        points->InsertNextPoint(fiberPoints[i].data());

        double ratio = static_cast<double>(i) / (fiberPoints.size() - 1);
        colors->InsertComponent(i, 0, static_cast<unsigned char>((1.0 - ratio) * 255));
        colors->InsertComponent(i, 1, 0);
        colors->InsertComponent(i, 2, static_cast<unsigned char>(ratio * 255));

        if(i > 0) {
            auto line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, i-1);
            line->GetPointIds()->SetId(1, i);
            cells->InsertNextCell(line);
        }
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(cells);
    polyData->GetPointData()->SetScalars(colors);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetLineWidth(2.0);

    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.1, 0.1);

    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(800, 800);
    renderWindow->SetWindowName("Single voxel VTK");

    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);

    auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor->SetInteractorStyle(style);

    renderWindow->Render();
    usleep(1000000);
    renderWindow->Finalize();
}