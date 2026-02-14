#pragma once

#include <nlohmann/json.hpp>
#include "detector_types.h"
#include "RGBController.h"

using json = nlohmann::json;

// forward decl
class DetectionManager;

#define DETECTOR_DISABLED_REASON "Detector is disabled"

class DetectorGroup
{
public:
    DetectorGroup(DetectionManager* detection_manager);

    virtual void Prepare() = 0;
    virtual void RunDetection() = 0;
    virtual void CleanUp() = 0;

    virtual std::string GroupName() = 0;
    virtual unsigned int GetNumDetectors() = 0;
    virtual std::vector<const std::string*> GetDetectorNames() const = 0;
    void UpdateSettings();

protected:

    json& GetDetectorSettings();
    bool IsDetectionEnabledFor(const std::string& detector_str);

    void RegisterRGBController(RGBController* ctrl);
    void SignalUpdate(unsigned int update_reason);

    void OnSkipDetection(const std::string& detector_str, const std::string& reason);
    void OnStartDetection(const std::string& detector_str);
    void OnEndDetection(const std::string& detector_str);

private:
    DetectionManager* detection_manager;
    json detector_settings;
};
