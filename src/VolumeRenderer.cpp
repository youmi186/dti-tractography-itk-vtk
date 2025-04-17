#include "VolumeRenderer.h"
#include <vtkNrrdReader.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageData.h>

VolumeRenderer::VolumeRenderer(const char* fname) : filename(fname) {
    SetupTransferFunctions();
    SetupVolume();
    SetupRenderer();
}

void VolumeRenderer::SetupTransferFunctions() {
    vtkSmartPointer<vtkNrrdReader> reader = vtkSmartPointer<vtkNrrdReader>::New();
    reader->SetFileName(filename);
    reader->Update();
    reader->GetOutput()->GetScalarRange(scalarRange);

    vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = 
        vtkSmartPointer<vtkPiecewiseFunction>::New();
    opacityTransferFunction->AddPoint(scalarRange[0], 0.0);
    opacityTransferFunction->AddPoint(scalarRange[1], 1.0);

    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = 
        vtkSmartPointer<vtkColorTransferFunction>::New();
    colorTransferFunction->AddRGBPoint(scalarRange[0], 0.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(scalarRange[1], 1.0, 1.0, 1.0);

    vtkSmartPointer<vtkVolumeProperty> volumeProperty = 
        vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetColor(colorTransferFunction);
    volumeProperty->SetScalarOpacity(opacityTransferFunction);
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();

    vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = 
        vtkSmartPointer<vtkSmartVolumeMapper>::New();
    volumeMapper->SetInputConnection(reader->GetOutputPort());

    volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);
}

void VolumeRenderer::SetupVolume() {
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(800, 800);

    renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
}

void VolumeRenderer::SetupRenderer() {
    renderer->AddVolume(volume);
    renderer->ResetCamera();
    renderer->SetBackground(1.0, 1.0, 1.0);
}

void VolumeRenderer::Render() {
    renderWindow->Render();
    renderWindowInteractor->Start();
}