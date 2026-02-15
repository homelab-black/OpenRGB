#include "hid_detector.h"

#include "LogManager.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "detector_types.h"
#ifdef __linux__
#include <unistd.h>
#endif

/*---------------------------------------------------------*\
| Default hidapi wrapper that just uses default hidapi     |
\*---------------------------------------------------------*/
const hidapi_wrapper default_hidapi_wrapper =
{
    NULL,
    (hidapi_wrapper_send_feature_report)            hid_send_feature_report,
    (hidapi_wrapper_get_feature_report)             hid_get_feature_report,
    (hidapi_wrapper_get_serial_number_string)       hid_get_serial_number_string,
    (hidapi_wrapper_open_path)                      hid_open_path,
    (hidapi_wrapper_enumerate)                      hid_enumerate,
    (hidapi_wrapper_free_enumeration)               hid_free_enumeration,
    (hidapi_wrapper_close)                          hid_close,
    (hidapi_wrapper_error)                          hid_error,
#if(HID_HOTPLUG_ENABLED)
    (hidapi_wrapper_hotplug_register_callback)      hid_hotplug_register_callback,
    (hidapi_wrapper_hotplug_deregister_callback)    hid_hotplug_deregister_callback,
#endif
};

HIDDetectorGroup::HIDDetectorGroup(DetectionManager* dm) : DetectorGroup(dm)
{
#if defined(__linux__) && defined(__GLIBC__)
    hidapi_libusb_handle            = nullptr;
#endif
}

HIDDetectorGroup::~HIDDetectorGroup()
{
    HIDExit();
}

std::string HIDDetectorGroup::GroupName()
{
    return "HID Detector";
}

unsigned int HIDDetectorGroup::GetNumDetectors()
{
    return hid_specific_detectors.size() + hid_generic_detectors.size() + hid_wrapped_specific_detectors.size() + hid_wrapped_generic_detectors.size();
}

std::vector<const std::string*> HIDDetectorGroup::GetDetectorNames() const
{
    std::vector<const std::string*> names;

    // todo: fix
    #define PUSH_NAMES(v) for (const auto& thing : v) { names.push_back(&thing.name); }

    PUSH_NAMES(hid_specific_detectors);
    PUSH_NAMES(hid_generic_detectors);
    PUSH_NAMES(hid_wrapped_specific_detectors);
    PUSH_NAMES(hid_wrapped_generic_detectors);
    return names;
}

void HIDDetectorGroup::RegisterHIDDeviceDetector(std::string name, HIDDeviceDetectorFunction  detector, int vid, int pid, int interface, int usage_page, int usage)
{
    HIDDeviceDetectorBlock block;

    block.name          = name;
    block.vid           = vid;
    block.pid           = pid;
    block.function      = detector;
    block.interface     = interface;
    block.usage_page    = usage_page;
    block.usage         = usage;

    if(block.vid == HID_VID_ANY && block.pid == HID_PID_ANY)
    {
        hid_generic_detectors.push_back(block);
    }
    else
    {
        hid_specific_detectors.push_back(block);
    }
}

void HIDDetectorGroup::RegisterHIDWrappedDeviceDetector(std::string name, HIDWrappedDeviceDetectorFunction  detector, int vid, int pid, int interface, int usage_page, int usage)
{
    HIDWrappedDeviceDetectorBlock block;

    block.name          = name;
    block.vid           = vid;
    block.pid           = pid;
    block.function      = detector;
    block.interface     = interface;
    block.usage_page    = usage_page;
    block.usage         = usage;

    if(block.vid == HID_VID_ANY && block.pid == HID_PID_ANY)
    {
        hid_wrapped_generic_detectors.push_back(block);
    }
    else
    {
        hid_wrapped_specific_detectors.push_back(block);
    }
}

void HIDDetectorGroup::Prepare()
{
    HIDInit();

    if(GetDetectorSettings().contains("hid_safe_mode"))
    {
        hid_safe_mode = GetDetectorSettings()["hid_safe_mode"];
    }
}

void HIDDetectorGroup::CleanUp()
{
#if(HID_HOTPLUG_ENABLED)
    if(!hid_safe_mode)
    {
        StopHIDHotplug();
    }
#endif
}

void HIDDetectorGroup::RunDetection()
{
    DetectedControllers found;
    if (hid_safe_mode)
    {
        found = RunSafeDetectection();
    }
    else
    {
        found = RunNormalDetection();
    }

#if defined(__linux__) && defined(__GLIBC__)
    DetectedControllers lib_usb_found = RunLibusbDetection();
    found.insert(found.end(), lib_usb_found.begin(), lib_usb_found.end());
#endif

    for (RGBController* ctrl : found)
    {
        RegisterRGBController(ctrl);
    }
}


DetectedControllers HIDDetectorGroup::RunSafeDetectection()
{
    DetectedControllers found = DetectedControllers {};

    // only create hid_device for known detectors
    for(HIDDeviceDetectorBlock& detector : hid_specific_detectors)
    {
        hid_device_info* hid_device = hid_enumerate(detector.vid, detector.pid);
        hid_device_info* current = hid_device;
        while(current)
        {
            DetectedControllers new_found = DetectWithDetector(hid_device, detector);
            if (!found.empty())
            {
                /*---------------------------------------*\
                | we can only find 1 device per detector? |
                \*---------------------------------------*/
                found.insert(found.end(), new_found.begin(), new_found.end());
                break;
            }
            current = current->next;
        }

        hid_free_enumeration(hid_device);
    }
    return found;
}

DetectedControllers HIDDetectorGroup::RunNormalDetection()
{
#if(HID_HOTPLUG_ENABLED)
    StopHIDHotplug();
    StartHIDHotplug();
#endif

    DetectedControllers found;
    hid_device_info* hid_device = hid_enumerate(0, 0);
    hid_device_info* current = hid_device;
    while (current)
    {
        DetectedControllers found1 = DetectNonWrapped(current);
        DetectedControllers found2 = DetectWrapped(current, (hidapi_wrapper*)&default_hidapi_wrapper);
        found.insert(found.end(), found1.begin(), found1.end());
        found.insert(found.end(), found2.begin(), found2.end());

        current = current->next;
    }
    hid_free_enumeration(hid_device);

    return found;
}

#if defined(__linux__) && defined(__GLIBC__)
DetectedControllers HIDDetectorGroup::RunLibusbDetection()
{
    DetectedControllers found;
    if (!hidapi_libusb_handle)
    {
        return found;
    }
    hid_device_info* hid_device = hidapi_libusb_wrapper.hid_enumerate(0, 0);
    hid_device_info* current = hid_device;
    while(current)
    {
        DetectedControllers new_found = DetectWrapped(current, &hidapi_libusb_wrapper);
        found.insert(found.end(), new_found.begin(), new_found.end());
        current = current->next;
    }
    return found;
}
#endif

DetectedControllers HIDDetectorGroup::DetectNonWrapped(hid_device_info* device)
{
    DetectedControllers found;
    bool skip_generic = false;
    for(HIDDeviceDetectorBlock& detector : hid_specific_detectors)
    {
        // we skip generics if at least one matching specific detector is on
        if (IsDetectionEnabledFor(detector.name) && detector.matching_id(device))
        {
            skip_generic = true;
        }
        DetectedControllers new_found = DetectWithDetector(device, detector);
        found.insert(found.end(), new_found.begin(), new_found.end());
    }

    if (!skip_generic)
    {
        for(HIDDeviceDetectorBlock& generic_detector : hid_generic_detectors)
        {
            DetectedControllers new_found = DetectWithDetector(device, generic_detector);
            found.insert(found.end(), new_found.begin(), new_found.end());
        }
    }
    return found;
}

DetectedControllers HIDDetectorGroup::DetectWrapped(hid_device_info* device, hidapi_wrapper* wrapper)
{
    DetectedControllers found;
    bool skip_generic = false;
    for(HIDWrappedDeviceDetectorBlock& detector : hid_wrapped_specific_detectors)
    {
        // we skip generics if at least one matching specific detector is on
        if (IsDetectionEnabledFor(detector.name) && detector.matching_id(device))
        {
            skip_generic = true;
        }
        detector.hid_wrapper = wrapper;
        DetectedControllers new_found = DetectWithDetector(device, detector);
        found.insert(found.end(), new_found.begin(), new_found.end());
    }

    if (!skip_generic)
    {
        for(HIDWrappedDeviceDetectorBlock& generic_detector : hid_wrapped_generic_detectors)
        {
            generic_detector.hid_wrapper = wrapper;
            DetectedControllers new_found = DetectWithDetector(device, generic_detector);
            found.insert(found.end(), new_found.begin(), new_found.end());
        }
    }
    return found;
}

DetectedControllers HIDDetectorGroup::DetectWithDetector(hid_device_info* device, BasicHIDBlock& detector)
{
    DetectedControllers found;

    if (!detector.compare(device))
    {
        return found;
    }

    if (!IsDetectionEnabledFor(detector.name))
    {
        OnSkipDetection(detector.name, DETECTOR_DISABLED_REASON);
        return found;
    }

    OnStartDetection(detector.name);
    found = detector.run(device);
    OnEndDetection(detector.name);

    for(std::size_t idx = 0; idx < found.size(); idx++)
    {
        found[idx]->SetDetectionPath(std::string(device->path));

#if(HID_HOTPLUG_ENABLED)
        int handle;
        hid_hotplug_register_callback(device->vendor_id, device->product_id, HID_API_HOTPLUG_EVENT_DEVICE_LEFT, 0, &DetectionManager::UnplugCallbackFunction, found[idx], &handle);
#endif
    }

    return found;
}

void HIDDetectorGroup::HIDInit()
{
    if (hid_initialised)
    {
        return;
    }
    hid_initialised = true;
    /*-----------------------------------------------------*\
    | Initialize HID interface                              |
    \*-----------------------------------------------------*/

    int hid_status = hid_init();

    LOG_DEBUG("[%s] Initializing HID interfaces: %s", GroupName().c_str(), ((hid_status == 0) ? "Success" : "Failed"));

#if defined(__linux__) && defined(__GLIBC__)
    /*-----------------------------------------------------*\
    | Load the libhidapi-libusb library                     |
    \*-----------------------------------------------------*/
#if(HID_HOTPLUG_ENABLED)
    hidapi_libusb_handle = dlopen("libhidapi-hotplug-libusb.so", RTLD_NOW | RTLD_NODELETE | RTLD_DEEPBIND);
#else
    hidapi_libusb_handle = dlopen("libhidapi-libusb.so", RTLD_NOW | RTLD_NODELETE | RTLD_DEEPBIND);
#endif

    if(hidapi_libusb_handle)
    {
        LOG_DEBUG("[%s] Loaded libusb HID library", GroupName().c_str());

        /*-------------------------------------------------*\
        | Create a wrapper with the libusb functions        |
        \*-------------------------------------------------*/
        hidapi_libusb_wrapper =
        {
            .dyn_handle                         = hidapi_libusb_handle,
            .hid_send_feature_report            = (hidapi_wrapper_send_feature_report)          dlsym(hidapi_libusb_handle,"hid_send_feature_report"),
            .hid_get_feature_report             = (hidapi_wrapper_get_feature_report)           dlsym(hidapi_libusb_handle,"hid_get_feature_report"),
            .hid_get_serial_number_string       = (hidapi_wrapper_get_serial_number_string)     dlsym(hidapi_libusb_handle,"hid_get_serial_number_string"),
            .hid_open_path                      = (hidapi_wrapper_open_path)                    dlsym(hidapi_libusb_handle,"hid_open_path"),
            .hid_enumerate                      = (hidapi_wrapper_enumerate)                    dlsym(hidapi_libusb_handle,"hid_enumerate"),
            .hid_free_enumeration               = (hidapi_wrapper_free_enumeration)             dlsym(hidapi_libusb_handle,"hid_free_enumeration"),
            .hid_close                          = (hidapi_wrapper_close)                        dlsym(hidapi_libusb_handle,"hid_close"),
            .hid_error                          = (hidapi_wrapper_error)                        dlsym(hidapi_libusb_handle,"hid_free_enumeration"),
#if(HID_HOTPLUG_ENABLED)
            .hid_hotplug_register_callback      = (hidapi_wrapper_hotplug_register_callback)    dlsym(hidapi_libusb_handle,"hid_hotplug_register_callback"),
            .hid_hotplug_deregister_callback    = (hidapi_wrapper_hotplug_deregister_callback)  dlsym(hidapi_libusb_handle,"hid_hotplug_deregister_callback"),
#endif
        };
    }
    else
    {
        LOG_DEBUG("[%s] Failed to load libusb HID library", GroupName().c_str());
    }
#endif
}

void HIDDetectorGroup::HIDExit()
{
    /*-----------------------------------------------------*\
    | Exit HID interface                                    |
    \*-----------------------------------------------------*/
    int hid_status = hid_exit();

    LOG_DEBUG("[%s] Exiting HID interface: %s", GroupName().c_str(), ((hid_status == 0) ? "Success" : "Failed"));
}

#if(HID_HOTPLUG_ENABLED)
/*---------------------------------------------------------*\
| HID hotplug management functions                          |
\*---------------------------------------------------------*/
void DetectionManager::StartHIDHotplug()
{
    hid_hotplug_register_callback(0, 0, HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED, HID_API_HOTPLUG_ENUMERATE, &DetectionManager::HotplugCallbackFunction, nullptr, &hotplug_callback_handle);

#if defined(__linux__) && defined(__GLIBC__)
    if(hidapi_libusb_handle != nullptr)
    {
        hidapi_libusb_wrapper.hid_hotplug_register_callback(0, 0, HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED, HID_API_HOTPLUG_ENUMERATE, &DetectionManager::WrappedHotplugCallbackFunction, nullptr, &libusb_hotplug_callback_handle);
    }
#endif
}

void DetectionManager::StopHIDHotplug()
{
    hid_hotplug_deregister_callback(hotplug_callback_handle);

#if defined(__linux__) && defined(__GLIBC__)
    if(hidapi_libusb_handle != nullptr)
    {
        hidapi_libusb_wrapper.hid_hotplug_deregister_callback(libusb_hotplug_callback_handle);
    }
#endif
}

/*---------------------------------------------------------*\
| HID hotplug callback functions                            |
\*---------------------------------------------------------*/
int DetectionManager::HotplugCallbackFunction(hid_hotplug_callback_handle callback_handle, hid_device_info *device, hid_hotplug_event event, void *user_data)
{
    /*-----------------------------------------------------*\
    | Open device disable list and read in disabled         |
    | device strings                                        |
    \*-----------------------------------------------------*/
    json                detector_settings   = ResourceManager::get()->GetSettingsManager()->GetSettings("Detectors");

    if(event == HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED)
    {
        LOG_INFO("[%s] HID device connected: [%04x:%04x - %s]", DETECTIONMANAGER, device->vendor_id, device->product_id, device->path);

        // todo: fix
        dm->RunHIDDetector(device, detector_settings);
        dm->RunHIDWrappedDetector(&default_hidapi_wrapper, device, detector_settings);
    }

    return 0;
}

int DetectionManager::UnplugCallbackFunction(hid_hotplug_callback_handle callback_handle, hid_device_info *device, hid_hotplug_event event, void *user_data)
{
    DetectionManager* dm = DetectionManager::get();

    if(event == HID_API_HOTPLUG_EVENT_DEVICE_LEFT)
    {
        LOG_INFO("[%s] HID device disconnected: [%04x:%04x - %s]", DETECTIONMANAGER, device->vendor_id, device->product_id, device->path);

        /*-------------------------------------------------*\
        | User data is the pointer to the controller being  |
        | removed                                           |
        \*-------------------------------------------------*/
        RGBController* controller = (RGBController*)(user_data);

        if(controller && controller->detection_path == std::string(device->path))
        {
            dm->UnregisterRGBController(controller);
            delete controller;
            return 1;
        }
    }
    return 0;
}

#if defined(__linux__) && defined(__GLIBC__)
int DetectionManager::WrappedHotplugCallbackFunction(hid_hotplug_callback_handle callback_handle, hid_device_info *device, hid_hotplug_event event, void *user_data)
{
    /*-----------------------------------------------------*\
    | Open device disable list and read in disabled         |
    | device strings                                        |
    \*-----------------------------------------------------*/
    json                detector_settings   = ResourceManager::get()->GetSettingsManager()->GetSettings("Detectors");
    DetectionManager*   dm                  = DetectionManager::get();

    if(event == HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED)
    {
        LOG_INFO("[%s] libusb HID device connected: [%04x:%04x - %s]", DETECTIONMANAGER, device->vendor_id, device->product_id, device->path);

        dm->RunHIDDetector(device, detector_settings);
        dm->RunHIDWrappedDetector(&DetectionManager::get()->hidapi_libusb_wrapper, device, detector_settings);
    }

    return 0;
}
#endif
#endif
