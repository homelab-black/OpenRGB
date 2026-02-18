#pragma once
#include "detector_group.h"
#include "detector_types.h"

class MiscDeviceDetectorGroup : public DetectorGroup
{
public:
    MiscDeviceDetectorGroup(DetectionManager* dm);
    ~MiscDeviceDetectorGroup();

    void Prepare() override;
    void RunDetection() override;
    void CleanUp() override;

    std::string GroupName() override;
    unsigned int GetNumDetectors() override;
    std::vector<const std::string*> GetDetectorNames() const override;

    void RegisterDeviceDetector(std::string name, DeviceDetectorFunction detector);

private:


    DetectedControllers DetectDevices();

    std::vector<DeviceDetectorBlock>         device_detectors;
};
