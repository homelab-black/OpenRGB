#pragma once
#include "detector_group.h"
#include "detector_types.h"

class MiscDeviceDetectorGroup : public DetectorGroup
{
public:
    MiscDeviceDetectorGroup(DetectionManager* dm);
    ~MiscDeviceDetectorGroup();

    virtual void Prepare() override;
    virtual void RunDetection() override;
    virtual void CleanUp() override;

    virtual std::string GroupName() override;
    virtual unsigned int GetNumDetectors() override;
    virtual std::vector<const std::string*> GetDetectorNames() const override;

    void RegisterDeviceDetector(std::string name, DeviceDetectorFunction detector);

private:


    DetectedControllers DetectDevices();

    std::vector<DeviceDetectorBlock>         device_detectors;
};
