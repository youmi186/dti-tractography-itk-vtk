#include "ComputeFAImage.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkVector.h"
#include <iostream>

using PixelType = float;
using VectorImageType = itk::VectorImage<PixelType, 3>;
using TensorImageType = itk::Image<itk::DiffusionTensor3D<PixelType>, 3>;
using FAImageType = itk::Image<PixelType, 3>;

void ComputeFAImage(const std::string &tensorImagePath, const std::string &faOutputPath)
{
    std::cout << "Task started: Computing FA image" << std::endl;

    // read DTI
    auto reader = itk::ImageFileReader<TensorImageType>::New();
    reader->SetFileName(tensorImagePath);
    reader->Update();

    // compute FA
    auto faFilter = itk::TensorFractionalAnisotropyImageFilter<TensorImageType, FAImageType>::New();
    faFilter->SetInput(reader->GetOutput());
    faFilter->Update();

    // save
    auto faWriter = itk::ImageFileWriter<FAImageType>::New();
    faWriter->SetFileName(faOutputPath);
    faWriter->SetInput(faFilter->GetOutput());
    faWriter->Update();

    std::cout << "Task completed: FA image saved to " << faOutputPath << std::endl;
}
