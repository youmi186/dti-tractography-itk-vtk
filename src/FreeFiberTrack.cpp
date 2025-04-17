#include "FreeFiberTrack.h"
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
#include <vtkCoordinate.h>
#include <vtkSphereSource.h>
#include <vtkLine.h>
#include <queue>
#include <fstream>
vtkStandardNewMacro(CustomInteractorStyle);

// 全局变量定义
std::array<double, 3> SeedPoint = {72, 72, 34};
bool SeedPointUpdated = false;

void CustomInteractorStyle::OnLeftButtonDown() {
    int* clickPos = this->GetInteractor()->GetEventPosition();

    vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(clickPos[0], clickPos[1], 0);

    double* worldPos = coordinate->GetComputedWorldValue(this->GetCurrentRenderer());
    SeedPoint = {worldPos[0], worldPos[1], worldPos[2]};
    SeedPointUpdated = true;
    printf("Seed point: [%.1f, %.1f, %.1f]\n", SeedPoint[0], SeedPoint[1], SeedPoint[2]);

    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

FreeFiberTrack::FreeFiberTrack(const char* vectorBinFile, const char* faFile)
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

std::array<double, 3> FreeFiberTrack::generateColor(int trackIndex) {
    const std::array<std::array<double, 3>, 6> colors = {{
        {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0},
        {1.0, 1.0, 0.0}, {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0}
    }};
    return colors[trackIndex % 6];
}

void FreeFiberTrack::setParameters(double newAlpha, double newStepSize) {
    alpha = newAlpha;
    stepSize = newStepSize;
}

bool FreeFiberTrack::isInside(const std::array<double, 3>& point) {
    return point[0] >= 0 && point[0] < dimensions[0] &&
           point[1] >= 0 && point[1] < dimensions[1] &&
           point[2] >= 0 && point[2] < dimensions[2];
}

double FreeFiberTrack::getFAValue(const std::array<double, 3>& point) {
    return faImage->GetScalarComponentAsDouble(
        static_cast<int>(point[0]),
        static_cast<int>(point[1]),
        static_cast<int>(point[2]),
        0
    );
}

std::array<double, 3> FreeFiberTrack::getVector(const std::array<double, 3>& point) {
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

void FreeFiberTrack::traceFiber(const std::array<double, 3>& seed) {
    printf("Seed point: [%.1f, %.1f, %.1f]\n", seed[0], seed[1], seed[2]);

    FiberTrack newTrack;
    newTrack.color = generateColor(fiberTracks.size());

    std::queue<std::array<double, 3>> q;
    q.push(seed);

    while (!q.empty() && newTrack.points.size() < MAX_STEPS) {
        auto currentPoint = q.front();
        q.pop();
        newTrack.points.push_back(currentPoint);
        auto vec = getVector(currentPoint);

        for (int direction : {-1, 1}) {
            std::array<double, 3> nextPoint;
            for (int i = 0; i < 3; i++) {
                nextPoint[i] = currentPoint[i] + stepSize * vec[i] * direction;
            }

            if (!isInside(nextPoint) || getFAValue(nextPoint) < alpha) {
                continue;
            }

            q.push(nextPoint);
        }
    }

    fiberTracks.push_back(newTrack);
}

void FreeFiberTrack::visualize() {
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.1, 0.1);

    for(const auto& track : fiberTracks) {
        // Create fiber line
        auto points = vtkSmartPointer<vtkPoints>::New();
        auto cells = vtkSmartPointer<vtkCellArray>::New();

        for(size_t i = 0; i < track.points.size(); i++) {
            points->InsertNextPoint(track.points[i].data());
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

        auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(polyData);

        auto actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(track.color[0], track.color[1], track.color[2]);
        actor->GetProperty()->SetLineWidth(2.0);
        renderer->AddActor(actor);

        // Add sphere at start point
        auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(track.points[0].data());
        sphere->SetRadius(1.0);

        auto sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        sphereMapper->SetInputConnection(sphere->GetOutputPort());

        auto sphereActor = vtkSmartPointer<vtkActor>::New();
        sphereActor->SetMapper(sphereMapper);
        sphereActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
        renderer->AddActor(sphereActor);
    }

    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(800, 800);
    renderWindow->SetWindowName("Single voxel VTK");

    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(renderWindow);

    auto style = vtkSmartPointer<CustomInteractorStyle>::New();
    interactor->SetInteractorStyle(style);

    renderWindow->Render();
    interactor->Start();
}