#ifndef VOLUME_RENDERER_H
#define VOLUME_RENDERER_H

#include <vtkSmartPointer.h>
#include <vtkVolume.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

class VolumeRenderer {
public:
    VolumeRenderer(const char* filename);
    void Render();

private:
    void SetupTransferFunctions();
    void SetupVolume();
    void SetupRenderer();

    vtkSmartPointer<vtkVolume> volume;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
    const char* filename;
    double scalarRange[2];
};

#endif