#include "detector_group.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "DetectionManager.h"


DetectorGroup::DetectorGroup(DetectionManager* detection_manager) : detection_manager(detection_manager)
{
}

void DetectorGroup::UpdateSettings()
{
    detector_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("Detectors");
}

bool DetectorGroup::IsDetectionEnabledFor(const std::string& detector_str)
{
    const json& settings = GetDetectorSettings();
    /*----------------------------------------------*\
    | This is why I want to do a settings rework btw |
    | I messed up the capitalisation here.....       |
    \*----------------------------------------------*/
    if (settings.contains("detectors") && settings["detectors"].contains(detector_str))
    {
        return settings["detectors"][detector_str];
    }
    return true;
}

void DetectorGroup::RegisterRGBController(RGBController* ctrl)
{
    detection_manager->RegisterRGBController(ctrl);
}

void DetectorGroup::SignalUpdate(DetectionUpdate update_reason)
{
    detection_manager->SignalUpdate(update_reason);
}

void DetectorGroup::OnSkipDetection(const std::string& detector_str, const std::string& reason)
{
    detection_manager->IncrementProcessedDetectorCount();
    detection_manager->SignalUpdate(DetectionUpdate::DetectionProgressChanged);
    LOG_TRACE("[%s] skpping %s: %s", GroupName().c_str(), detector_str.c_str(), reason.c_str());
}

void DetectorGroup::OnStartDetection(const std::string& detector_str)
{
    detection_manager->SetDetectorString(detector_str);
    detection_manager->SignalUpdate(DetectionUpdate::DetectionProgressChanged);
    LOG_DEBUG("[%s] %s detection start", GroupName().c_str(), detector_str.c_str());
}

void DetectorGroup::OnEndDetection(const std::string& detector_str)
{
    detection_manager->IncrementProcessedDetectorCount();
    detection_manager->SignalUpdate(DetectionUpdate::DetectionProgressChanged);
    LOG_DEBUG("[%s] %s detection end", GroupName().c_str(), detector_str.c_str());
}

void DetectorGroup::IncrementProcessedCount()
{
    IncrementProcessedCount(1);
}

void DetectorGroup::IncrementProcessedCount(unsigned int count)
{
    detection_manager->IncrementProcessedDetectorCount(count);
    detection_manager->SignalUpdate(DetectionUpdate::DetectionProgressChanged);
}

json& DetectorGroup::GetDetectorSettings()
{
    return detector_settings;
}
