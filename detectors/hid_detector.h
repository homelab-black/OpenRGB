#pragma once

#include "detector_group.h"
#include "detector_types.h"
#include "hidapi.h"
#include "hidapi_wrapper.h"

#if(HID_HOTPLUG_ENABLED)
#include "dependencies/hidapi-hotplug-win/include/hidapi.h"
#endif

class HIDDetectorGroup : public DetectorGroup
{
public:
    HIDDetectorGroup(DetectionManager* detection_manager);
    ~HIDDetectorGroup();

    void Prepare() override;
    void RunDetection() override;
    void CleanUp() override;

    std::string GroupName() override;
    unsigned int GetNumDetectors() override;
    std::vector<const std::string*> GetDetectorNames() const override;

    void RegisterHIDDeviceDetector(std::string name, HIDDeviceDetectorFunction  detector, int vid, int pid, int interface, int usage_page, int usage);
    void RegisterHIDWrappedDeviceDetector(std::string name, HIDWrappedDeviceDetectorFunction  detector, int vid, int pid, int interface, int usage_page, int usage);

private:

    void HIDInit();
    void HIDExit();

    DetectedControllers RunNormalDetection();
    DetectedControllers RunSafeDetectection();
#if defined(__linux__) && defined(__GLIBC__)
    DetectedControllers RunLibusbDetection();
#endif
    DetectedControllers DetectNonWrapped(hid_device_info* device);
    DetectedControllers DetectWrapped(hid_device_info* device, hidapi_wrapper* wrapper);

    /*----------------------------*\
    | Runs detector using a device |
    \*----------------------------*/
    DetectedControllers DetectWithDetector(hid_device_info* device, BasicHIDBlock& detector);


    hid_device_info* hid_devices = NULL; // only valid after PrepareDetection() and before CleanUp();
    bool hid_safe_mode;
    bool hid_initialised = false;

    std::vector<HIDDeviceDetectorBlock>         hid_generic_detectors;
    std::vector<HIDDeviceDetectorBlock>         hid_specific_detectors;
    std::vector<HIDWrappedDeviceDetectorBlock>  hid_wrapped_generic_detectors;
    std::vector<HIDWrappedDeviceDetectorBlock>  hid_wrapped_specific_detectors;

#if defined(__linux__) && defined(__GLIBC__)

    /*-----------------------------------------------------*\
    | Wrapped libusb hidapi handles                         |
    \*-----------------------------------------------------*/
    void *                                      hidapi_libusb_handle;
    hidapi_wrapper                              hidapi_libusb_wrapper;
#endif

/*-----------------------------------------------------*\
| HID Hotplug handles                                   |
\*-----------------------------------------------------*/
#if(HID_HOTPLUG_ENABLED)
    hid_hotplug_callback_handle                 hotplug_callback_handle;
#if defined(__linux__) && defined(__GLIBC__)
    hid_hotplug_callback_handle                 libusb_hotplug_callback_handle;
#endif
#endif
};
