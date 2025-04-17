#include "ComputePrincipalEigenvector.h"
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkDiffusionTensor3D.h>
#include <itkVector.h>
#include <itkImage.h>
#include <itkSymmetricEigenAnalysisImageFilter.h>
#include <iostream>

using PixelType = itk::DiffusionTensor3D<double>;
using ImageType = itk::Image<PixelType, 3>;
using VectorType = itk::Vector<double, 3>;
using VectorImageType = itk::Image<VectorType, 3>;

void ComputePrincipalEigenvector(const std::string &tensorImagePath, const std::string &outputImagePath)
{
    std::cout << "Task started: Computing principal eigenvector" << std::endl;

    // read DTI
    using ReaderType = itk::ImageFileReader<ImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(tensorImagePath);
    reader->Update();

    ImageType::Pointer tensorImage = reader->GetOutput();

    // create output image
    VectorImageType::Pointer vectorImage = VectorImageType::New();
    vectorImage->SetRegions(tensorImage->GetLargestPossibleRegion());
    vectorImage->SetSpacing(tensorImage->GetSpacing());
    vectorImage->SetOrigin(tensorImage->GetOrigin());
    vectorImage->SetDirection(tensorImage->GetDirection());
    vectorImage->Allocate();

    // eigen calculator setup
    using MatrixType = itk::Matrix<double, 3, 3>;
    using EigenValuesArrayType = itk::FixedArray<double, 3>;
    using EigenVectorsMatrixType = itk::Matrix<double, 3, 3>;
    itk::SymmetricEigenAnalysis<MatrixType, EigenValuesArrayType, EigenVectorsMatrixType> eigenCalculator;
    eigenCalculator.SetDimension(3);

    // compute eigenvectors
    itk::ImageRegionIterator<ImageType> tensorIt(tensorImage, tensorImage->GetLargestPossibleRegion());
    itk::ImageRegionIterator<VectorImageType> vectorIt(vectorImage, vectorImage->GetLargestPossibleRegion());

    for (tensorIt.GoToBegin(), vectorIt.GoToBegin(); !tensorIt.IsAtEnd(); ++tensorIt, ++vectorIt) {
        PixelType tensor = tensorIt.Get();
        MatrixType tensorMatrix;

        tensorMatrix[0][0] = tensor[0];
        tensorMatrix[0][1] = tensor[1];
        tensorMatrix[0][2] = tensor[2];
        tensorMatrix[1][0] = tensor[1];
        tensorMatrix[1][1] = tensor[3];
        tensorMatrix[1][2] = tensor[4];
        tensorMatrix[2][0] = tensor[2];
        tensorMatrix[2][1] = tensor[4];
        tensorMatrix[2][2] = tensor[5];

        EigenValuesArrayType eigenValues;
        EigenVectorsMatrixType eigenVectors;

        eigenCalculator.ComputeEigenValuesAndVectors(tensorMatrix, eigenValues, eigenVectors);

        unsigned int maxIndex = std::distance(eigenValues.begin(), std::max_element(eigenValues.begin(), eigenValues.end()));

        VectorType principalEigenvector;
        for (unsigned int i = 0; i < 3; ++i) {
            principalEigenvector[i] = eigenVectors[i][maxIndex];
        }

        vectorIt.Set(principalEigenvector);
    }

    // Save
    using WriterType = itk::ImageFileWriter<VectorImageType>;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(outputImagePath);
    writer->SetInput(vectorImage);
    writer->Update();

    std::cout << "Task completed: Principal eigenvector image saved to " << outputImagePath << std::endl;
}
