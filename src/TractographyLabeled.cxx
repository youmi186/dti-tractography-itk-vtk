#include "TractographyLabeled.h"
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkDiffusionTensor3D.h>
#include <itkVector.h>
#include <itkImage.h>
#include <queue>
#include <iostream>

using PixelType = itk::DiffusionTensor3D<double>;
using ImageType = itk::Image<PixelType, 3>;
using VectorType = itk::Vector<double, 3>;
using VectorImageType = itk::Image<VectorType, 3>;
using FAImageType = itk::Image<double, 3>;
using LabelImageType = itk::Image<unsigned char, 3>;
using OutputImageType = itk::Image<unsigned int, 3>;
using IndexType = ImageType::IndexType;

void PerformTractographyLabeled(const std::string &tensorImagePath, const std::string &faImagePath, const std::string &labelImagePath, const std::string &eigenvectorImagePath, const std::string &outputImagePath)
{
    std::cout << "Task started: Tractography - Labeled" << std::endl;
	
	// read principal eigenvector image
    auto eigenvectorReader = itk::ImageFileReader<VectorImageType>::New();
    eigenvectorReader->SetFileName(eigenvectorImagePath);
    eigenvectorReader->Update();
	
    // read FA
    auto faReader = itk::ImageFileReader<FAImageType>::New();
    faReader->SetFileName(faImagePath);
    faReader->Update();

    // read label image
    auto labelReader = itk::ImageFileReader<LabelImageType>::New();
    labelReader->SetFileName(labelImagePath);
    labelReader->Update();

    // get image region and pointers
    FAImageType::Pointer faImage = faReader->GetOutput();
    LabelImageType::Pointer labelImage = labelReader->GetOutput();
    VectorImageType::Pointer eigenvectorImage = eigenvectorReader->GetOutput();
    FAImageType::RegionType region = faImage->GetLargestPossibleRegion();

    // initialize output image
    OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetRegions(region);
    outputImage->SetSpacing(faImage->GetSpacing());
    outputImage->SetOrigin(faImage->GetOrigin());
    outputImage->SetDirection(faImage->GetDirection());
    outputImage->Allocate();
    outputImage->FillBuffer(0);

    // initialize tracking queue
    std::queue<IndexType> visitQueue;
    unsigned int stepCount = 1;
    const double minFA = 0.5;
    const unsigned int maxSteps = 20000;
    const double stepSize = 1;
	
    itk::ImageRegionIterator<LabelImageType> labelIt(labelImage, region);
    for (labelIt.GoToBegin(); !labelIt.IsAtEnd(); ++labelIt) {
        if (labelIt.Get() == 1) {
            IndexType seedIndex = labelIt.GetIndex();
            visitQueue.push(seedIndex);
            outputImage->SetPixel(seedIndex, stepCount);
        }
    }

    // Algorithm
    while (!visitQueue.empty() && stepCount < maxSteps) {
        IndexType currentIndex = visitQueue.front();
        visitQueue.pop();
		
        if (faImage->GetPixel(currentIndex) < minFA) continue;

        VectorType principalEigenvector = eigenvectorImage->GetPixel(currentIndex);

        for (int sign : {1, -1}) {
            IndexType nextIndex = currentIndex;
            for (unsigned int i = 0; i < 3; ++i) {
                nextIndex[i] += sign * static_cast<int>(std::round(principalEigenvector[i] * stepSize));
            }

            if (region.IsInside(nextIndex) && outputImage->GetPixel(nextIndex) == 0 &&
                faImage->GetPixel(nextIndex) >= minFA) {
                visitQueue.push(nextIndex);
                outputImage->SetPixel(nextIndex, ++stepCount);
            }
        }
    }

    std::cout << "TractographyLabeled completed. Steps taken: " << stepCount << std::endl;

    // save
    auto writer = itk::ImageFileWriter<OutputImageType>::New();
    writer->SetFileName(outputImagePath);
    writer->SetInput(outputImage);
    writer->Update();

    std::cout << "TractographyLabeled output saved to " << outputImagePath << std::endl;
}
