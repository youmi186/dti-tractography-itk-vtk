#include "VolumeRenderer.h"
#include "SingleSeedFiberTrack.h"
#include "LabeledFiberTrack.h"
#include "FreeFiberTrack.h"

int main(int argc, char* argv[]) {

    const char* vectorBinFile = "../data/eigenvector_data.bin";
    const char* faFile = "../data/FA.nrrd";
    const char* labelFile = "../data/FALabeled.nrrd";

    // 1. Volume Rendering
    VolumeRenderer renderer("../data/FA.nrrd");
	renderer.Render();

    // 2. Single Seed Fiber Tracking
    SingleSeedFiberTrack singleFiber(vectorBinFile, faFile);
    singleFiber.setParameters(0.3, 1.5);
    std::array<double, 3> seed = {72.0, 72.0, 34.0};
    singleFiber.traceFiber(seed);
    singleFiber.visualize();

    // 3. Labeled Fiber Tracking
    LabeledFiberTrack labeledFiber(vectorBinFile, faFile);
    labeledFiber.setParameters(0.3, 1.5);
    labeledFiber.traceAllFibers(labelFile);
    labeledFiber.visualize();

    // 4. Free Fiber Tracking
    FreeFiberTrack freeFiber(vectorBinFile, faFile);
    freeFiber.setParameters(0.5, 0.8);
    while (true) {
        freeFiber.traceFiber(SeedPoint);
        freeFiber.visualize();
        if (!SeedPointUpdated) {
            break;
        }
    }

    return 0;
}