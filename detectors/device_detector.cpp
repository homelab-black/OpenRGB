#include "device_detector.h"
#include "LogManager.h"

MiscDeviceDetectorGroup::MiscDeviceDetectorGroup(DetectionManager* dm) : DetectorGroup(dm)
{
}

MiscDeviceDetectorGroup::~MiscDeviceDetectorGroup()
{
}

std::string MiscDeviceDetectorGroup::GroupName()
{
    return "Misc Devices Detector";
}

unsigned int MiscDeviceDetectorGroup::GetNumDetectors()
{
    return device_detectors.size();
}

std::vector<const std::string*> MiscDeviceDetectorGroup::GetDetectorNames() const
{
    std::vector<const std::string*> names;
    for(const auto& detector : device_detectors)
    {
        names.push_back(&detector.name);
    }
    return names;
}


void MiscDeviceDetectorGroup::RegisterDeviceDetector(std::string name, DeviceDetectorFunction detector)
{
    DeviceDetectorBlock block
    {
        .name = name,
        .detector = detector,
    };
    device_detectors.push_back(block);
}

void MiscDeviceDetectorGroup::Prepare()
{
}

void MiscDeviceDetectorGroup::CleanUp()
{
}

void MiscDeviceDetectorGroup::RunDetection()
{
    DetectedControllers found = DetectDevices();
    for (RGBController* ctrl : found)
    {
        RegisterRGBController(ctrl);
    }
}

DetectedControllers MiscDeviceDetectorGroup::DetectDevices()
{
    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|               Detecting misc devices               |");
    LOG_INFO("------------------------------------------------------");

    OnStartDetection("Detecting miscellaneous Devices");

    DetectedControllers found;

    for (DeviceDetectorBlock& detector : device_detectors)
    {
        if (!IsDetectionEnabledFor(detector.name))
        {
            OnSkipDetection(detector.name, DETECTOR_DISABLED_REASON);
            continue;
        }

        OnStartDetection(detector.name);
        DetectedControllers new_found = detector.detector();
        found.insert(found.end(), new_found.begin(), new_found.end());
        OnEndDetection(detector.name);
    }
    return found;
}
