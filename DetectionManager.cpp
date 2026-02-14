/*---------------------------------------------------------*\
| DetectionManager.cpp                                      |
|                                                           |
|   OpenRGB Detection Manager handles detecting supported   |
|   devices and initializing their RGBControllers           |
|                                                           |
|   Adam Honse <calcprogrammer1@gmail.com>      08 Jan 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include "DetectionManager.h"
#include "LogManager.h"
#include "pci_ids.h"
#include "ProfileManager.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "StringUtils.h"

#ifdef __linux__
#include <unistd.h>
#endif

using namespace std::chrono_literals;

/*---------------------------------------------------------*\
| DetectionManager name for log entries                     |
\*---------------------------------------------------------*/
const char* DETECTIONMANAGER = "DetectionManager";

/*---------------------------------------------------------*\
| Define a macro for QT lupdate to parse                    |
\*---------------------------------------------------------*/
#define QT_TRANSLATE_NOOP(scope, x) x

/*---------------------------------------------------------*\
| Warning Strings                                           |
\*---------------------------------------------------------*/
const char* I2C_ERR_WIN =   QT_TRANSLATE_NOOP("DetectionManager",
                                              "<h2>Some internal devices may not be detected:</h2>"
                                              "<p>One or more I2C or SMBus interfaces failed to initialize.</p>"
                                              "<p><b>RGB DRAM modules, some motherboards' onboard RGB lighting, and RGB Graphics Cards, will not be available in OpenRGB</b> without I2C or SMBus.</p>"
                                              "<h4>How to fix this:</h4>"
                                              "<p>On Windows, this is usually caused by a failure to load the PawnIO driver.</p>"
                                              "<p>You must first install <a href='https://pawnio.eu/'>PawnIO</a>, then you must OpenRGB as administrator in order to access these devices.</p>"
                                              "<p>See <a href='https://help.openrgb.org/'>help.openrgb.org</a> for additional troubleshooting steps if you keep seeing this message.<br></p>"
                                              "<h3>If you are not using internal RGB on a desktop this message is not important to you.</h3>");
const char* I2C_ERR_LINUX = QT_TRANSLATE_NOOP("DetectionManager",
                                              "<h2>Some internal devices may not be detected:</h2>"
                                              "<p>One or more I2C or SMBus interfaces failed to initialize.</p>"
                                              "<p><b>RGB DRAM modules, some motherboards' onboard RGB lighting, and RGB Graphics Cards, will not be available in OpenRGB</b> without I2C or SMBus.</p>"
                                              "<h4>How to fix this:</h4>"
                                              "<p>On Linux, this is usually because the i2c-dev module is not loaded.</p>"
                                              "<p>You must load the i2c-dev module along with the correct i2c driver for your motherboard. "
                                              "This is usually i2c-piix4 for AMD systems and i2c-i801 for Intel systems.</p>"
                                              "<p>See <a href='https://help.openrgb.org/'>help.openrgb.org</a> for additional troubleshooting steps if you keep seeing this message.<br></p>"
                                              "<h3>If you are not using internal RGB on a desktop this message is not important to you.</h3>");

const char* UDEV_MISSING =  QT_TRANSLATE_NOOP("DetectionManager",
                                              "<h2>WARNING:</h2>"
                                              "<p>The OpenRGB udev rules are not installed.</p>"
                                              "<p>Most devices will not be available unless running OpenRGB as root.</p>"
                                              "<p>If using AppImage, Flatpak, or self-compiled versions of OpenRGB you must install the udev rules manually</p>"
                                              "<p>See <a href='https://openrgb.org/udev'>https://openrgb.org/udev</a> to install the udev rules manually</p>");
const char* UDEV_MUTLI =    QT_TRANSLATE_NOOP("DetectionManager",
                                              "<h2>WARNING:</h2>"
                                              "<p>Multiple OpenRGB udev rules are installed.</p>"
                                              "<p>The udev rules file 60-openrgb.rules is installed in both /etc/udev/rules.d and /usr/lib/udev/rules.d.</p>"
                                              "<p>Multiple udev rules files can conflict, it is recommended to remove one of them.</p>");



/*---------------------------------------------------------*\
| DetectionManager Global Instance Pointer                  |
\*---------------------------------------------------------*/
DetectionManager* DetectionManager::instance;

DetectionManager::DetectionManager()
: i2c_group(this), hid_group(this), others_group(this)
{
    /*-----------------------------------------------------*\
    | Initialize global instance pointer the when created   |
    | There should only ever be one instance of             |
    | DetectionManager                                      |
    \*-----------------------------------------------------*/
    if(!instance)
    {
        instance = this;
    }
    /*-----------------------------------------------------*\
    | If, for whatever reason, DetectionManager already     |
    | exists, delete this instance as only one should exist |
    \*-----------------------------------------------------*/
    else
    {
        delete this;
        return;
    }

    /*-----------------------------------------------------*\
    | Initialize variables                                  |
    \*-----------------------------------------------------*/
    detection_in_progress           = false;
    detection_percent               = 100;
    detection_percent_denominator   = 0;
    detection_string                = "";
    dynamic_detectors_processed     = false;
    initial_detection               = true;

    /*-----------------------------------------------------*\
    | Start the background thread                           |
    \*-----------------------------------------------------*/
    background_thread_running       = true;
    BackgroundThread                = new std::thread(&DetectionManager::BackgroundThreadFunction, this);
}

DetectionManager::~DetectionManager()
{
    /*-----------------------------------------------------*\
    | Exit HID in background thread                         |
    \*-----------------------------------------------------*/
    // RunInBackgroundThread(std::bind(&DetectionManager::BackgroundHIDExit, this));

    /*-----------------------------------------------------*\
    | Mark the background detection thread as not running   |
    | and then wake it up so it knows that it has to stop   |
    \*-----------------------------------------------------*/
    background_thread_running = false;
    BackgroundThreadScheduledFunctionStart.notify_one();

    /*-----------------------------------------------------*\
    | Stop the background thread                            |
    \*-----------------------------------------------------*/
    if(BackgroundThread)
    {
        BackgroundThread->join();
        delete BackgroundThread;
        BackgroundThread = nullptr;
    }
}

/*---------------------------------------------------------*\
| DetectionManager Global Instance Accessor                 |
\*---------------------------------------------------------*/
DetectionManager* DetectionManager::get()
{
    /*-----------------------------------------------------*\
    | If DetectionManager does not exist yet, create it     |
    \*-----------------------------------------------------*/
    if(!instance)
    {
        instance = new DetectionManager();
    }

    return instance;
}

/*---------------------------------------------------------*\
| Functions to access device lists                          |
\*---------------------------------------------------------*/
std::vector<i2c_smbus_interface*>& DetectionManager::GetI2CBuses()
{
    return(i2c_group.GetI2CBuses());
}

std::vector<RGBController*>& DetectionManager::GetRGBControllers()
{
    return(rgb_controllers);
}

/*---------------------------------------------------------*\
| Functions for registering I2C buses from within detectors |
\*---------------------------------------------------------*/
void DetectionManager::RegisterI2CBus(i2c_smbus_interface *bus)
{
    i2c_group.RegisterI2CBus(bus);
}

/*---------------------------------------------------------*\
| I2C Bus Detector Registration Function                    |
\*---------------------------------------------------------*/
void DetectionManager::RegisterI2CBusDetector(I2CBusDetectorFunction detector)
{
    i2c_group.RegisterI2CBusDetector(detector);
}

/*---------------------------------------------------------*\
| RGBController Detector Registration Functions             |
\*---------------------------------------------------------*/
void DetectionManager::RegisterDeviceDetector(std::string name, DeviceDetectorFunction detector)
{
    others_group.RegisterDeviceDetector(name, detector);
}

void DetectionManager::RegisterDynamicDetector(std::string name, DynamicDetectorFunction detector)
{
    DynamicDetectorBlock block
    {
        .name = name,
        .detector = detector,
    };
    dynamic_detectors.push_back(block);
}

void DetectionManager::RegisterHIDDeviceDetector(std::string name, HIDDeviceDetectorFunction  detector, int vid, int pid, int interface, int usage_page, int usage)
{
    hid_group.RegisterHIDDeviceDetector(name, detector, vid, pid, interface, usage_page, usage);
}

void DetectionManager::RegisterHIDWrappedDeviceDetector(std::string name, HIDWrappedDeviceDetectorFunction  detector, int vid, int pid, int interface, int usage_page, int usage)
{
    hid_group.RegisterHIDWrappedDeviceDetector(name, detector, vid, pid, interface, usage_page, usage);
}

void DetectionManager::RegisterI2CDeviceDetector(std::string name, I2CDeviceDetectorFunction detector)
{
    i2c_group.RegisterI2CDeviceDetector(name, detector);
}

void DetectionManager::RegisterI2CDRAMDeviceDetector(std::string name, I2CDRAMDeviceDetectorFunction detector, uint16_t jedec_id, uint8_t dram_type)
{
    i2c_group.RegisterI2CDRAMDeviceDetector(name, detector, jedec_id, dram_type);
}

void DetectionManager::RegisterI2CPCIDeviceDetector(std::string name, I2CPCIDeviceDetectorFunction detector, uint16_t ven_id, uint16_t dev_id, uint16_t subven_id, uint16_t subdev_id, uint8_t i2c_addr)
{
    i2c_group.RegisterI2CPCIDeviceDetector(name, detector, ven_id, dev_id, subven_id, subdev_id, i2c_addr);
}

/*---------------------------------------------------------*\
| Pre-Detection Hook Function Registration Function         |
\*---------------------------------------------------------*/
void DetectionManager::RegisterPreDetectionHook(PreDetectionHookFunction hook)
{
    pre_detection_hooks.push_back(hook);
}

/*---------------------------------------------------------*\
| Detection Callback Registration Functions                 |
\*---------------------------------------------------------*/
void DetectionManager::RegisterDetectionCallback(DetectionCallback new_callback, void * new_callback_arg)
{
    std::unique_lock<std::mutex> lock(DetectionCallbackMutex);

    for(std::size_t idx = 0; idx < DetectionCallbacks.size(); idx++)
    {
        if(DetectionCallbacks[idx] == new_callback && DetectionCallbackArgs[idx] == new_callback_arg)
        {
            LOG_TRACE("[%s] Tried to register an already registered detection callback, skipping.  Total callbacks registered: %d", DETECTIONMANAGER, DetectionCallbacks.size());
            return;
        }
    }

    DetectionCallbacks.push_back(new_callback);
    DetectionCallbackArgs.push_back(new_callback_arg);

    LOG_TRACE("[%s] Registered detection callback.  Total callbacks registered: %d", DETECTIONMANAGER, DetectionCallbacks.size());
}

void DetectionManager::UnregisterDetectionCallback(DetectionCallback callback, void *callback_arg)
{
    std::unique_lock<std::mutex> lock(DetectionCallbackMutex);

    for(std::size_t idx = 0; idx < DetectionCallbacks.size(); idx++)
    {
        if(DetectionCallbacks[idx] == callback && DetectionCallbackArgs[idx] == callback_arg)
        {
            DetectionCallbacks.erase(DetectionCallbacks.begin() + idx);
            DetectionCallbackArgs.erase(DetectionCallbackArgs.begin() + idx);
        }
    }

    LOG_TRACE("[%s] Unregistered detection callback.  Total callbacks registered: %d", DETECTIONMANAGER, DetectionCallbacks.size());
}

/*---------------------------------------------------------*\
| Functions for registering RGBControllers from within      |
| detectors                                                 |
\*---------------------------------------------------------*/
void DetectionManager::RegisterRGBController(RGBController *rgb_controller)
{
    LOG_INFO("[%s] Registering RGB controller %s", DETECTIONMANAGER, rgb_controller->GetName().c_str());

    /*-----------------------------------------------------*\
    | Mark this controller as locally owned                 |
    \*-----------------------------------------------------*/
    rgb_controller->flags &= ~CONTROLLER_FLAG_REMOTE;
    rgb_controller->flags |= CONTROLLER_FLAG_LOCAL;

    /*-----------------------------------------------------*\
    | Load sizes for the new controller                     |
    \*-----------------------------------------------------*/
    ResourceManager::get()->GetProfileManager()->LoadControllerFromListWithOptions(rgb_controllers_sizes, detection_size_entry_used, rgb_controller, true, false);

    /*-----------------------------------------------------*\
    | Add the new controller to the list                    |
    \*-----------------------------------------------------*/
    rgb_controllers.push_back(rgb_controller);

    /*-----------------------------------------------------*\
    | Signal Device List Update                             |
    \*-----------------------------------------------------*/
    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_RGBCONTROLLER_REGISTERED);
}

void DetectionManager::UnregisterRGBController(RGBController* rgb_controller)
{
    LOG_INFO("[%s] Unregistering RGB controller %s", DETECTIONMANAGER, rgb_controller->GetName().c_str());

    /*-----------------------------------------------------*\
    | Clear callbacks from the controller before removal    |
    \*-----------------------------------------------------*/
    rgb_controller->ClearCallbacks();

    /*-----------------------------------------------------*\
    | Find the controller to remove and remove it from the  |
    | master list                                           |
    \*-----------------------------------------------------*/
    std::vector<RGBController*>::iterator rgb_it = std::find(rgb_controllers.begin(), rgb_controllers.end(), rgb_controller);

    if(rgb_it != rgb_controllers.end())
    {
        rgb_controllers.erase(rgb_it);
    }

    /*-----------------------------------------------------*\
    | Signal Device List Update                             |
    \*-----------------------------------------------------*/
    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_RGBCONTROLLER_UNREGISTERED);
}

/*---------------------------------------------------------*\
| Detection state functions                                 |
\*---------------------------------------------------------*/
void DetectionManager::AbortDetection()
{
    LOG_INFO("[%s] Detection abort requested", DETECTIONMANAGER);

    detection_in_progress   = false;
    detection_percent       = 100;
    detection_string        = "Stopping";
}

void DetectionManager::BeginDetection()
{
    /*-----------------------------------------------------*\
    | Perform pre-detection setup                           |
    \*-----------------------------------------------------*/
    bool detection_ready = ProcessPreDetection();

    /*-----------------------------------------------------*\
    | Run DetectDevices function in background thread if    |
    | ready to run detection                                |
    \*-----------------------------------------------------*/
    if(detection_ready)
    {
        RunInBackgroundThread(std::bind(&DetectionManager::BackgroundDetectDevices, this));
    }
}

unsigned int DetectionManager::GetDetectionPercent()
{
    return(detection_percent.load());
}

std::string DetectionManager::GetDetectionString()
{
    return(detection_string);
}

void DetectionManager::WaitForDetection()
{
    DetectDevicesMutex.lock();
    DetectDevicesMutex.unlock();
}

/*---------------------------------------------------------*\
| DetectionManager Background Thread Functions              |
\*---------------------------------------------------------*/
void DetectionManager::BackgroundThreadFunction()
{
    /*-----------------------------------------------------*\
    | The background thread that runs scheduled coroutines  |
    | when applicable                                       |
    | Stays asleep if nothing is scheduled                  |
    | NOTE: this thread owns the HIDAPI library internal    |
    | objects on MacOS                                      |
    | hid_init and hid_exit may not be called outside of    |
    | this thread.  Calling hid_exit outside of this thread |
    | WILL cause an immediate CRASH on MacOS.               |
    | BackgroundThreadStateMutex will be UNLOCKED as long   |
    | as the thread is suspended.  It locks automatically   |
    | when any coroutine is running.  However, it seems to  |
    | be necessary to be separate from the                  |
    | DeviceDetectionMutex, even though their states are    |
    | nearly identical.                                     |
    \------------------------------------------------------*/
    std::unique_lock lock(BackgroundThreadStateMutex);

    while(background_thread_running)
    {
        if(BackgroundThreadScheduledFunction)
        {
            std::function<void()> coroutine = nullptr;
            std::swap(BackgroundThreadScheduledFunction, coroutine);

            try
            {
                coroutine();
            }
            catch(std::exception& e)
            {
                LOG_ERROR("[%s] Unhandled exception in coroutine; e.what(): %s", DETECTIONMANAGER, e.what());
            }
            catch(...)
            {
                LOG_ERROR("[%s] Unhandled exception in coroutine", DETECTIONMANAGER);
            }
        }

        /*-------------------------------------------------*\
        | This line will cause the thread to suspend until  |
        | the condition variable is triggered               |
        | NOTE: it may be subject to "spurious wakeups"     |
        \*-------------------------------------------------*/
        BackgroundThreadScheduledFunctionStart.wait(lock);
    }
}

void DetectionManager::RunInBackgroundThread(std::function<void()> coroutine)
{
    if(std::this_thread::get_id() == BackgroundThread->get_id())
    {
        /*-------------------------------------------------*\
        | We are already in the background thread - don't   |
        | schedule the call, run it immediately             |
        \*-------------------------------------------------*/
        coroutine();
    }
    else
    {
        /*-------------------------------------------------*\
        | Lock the background thread state mutex and assign |
        | the new function to the background thread, then   |
        | trigger the background thread to start it         |
        \*-------------------------------------------------*/
        BackgroundThreadStateMutex.lock();

        if(BackgroundThreadScheduledFunction != nullptr)
        {
            LOG_WARNING("[%s] Detection coroutine: assigned a new coroutine when one was already scheduled - probably two rescan events sent at once", DETECTIONMANAGER);
        }

        BackgroundThreadScheduledFunction = coroutine;
        BackgroundThreadStateMutex.unlock();
        BackgroundThreadScheduledFunctionStart.notify_one();
    }
}

/*---------------------------------------------------------*\
| Functions that must be run in the background thread       |
\*---------------------------------------------------------*/
void DetectionManager::BackgroundDetectDevices()
{
    LOG_DEBUG("start detect device");

    /*-----------------------------------------------------*\
    | Lock detection mutex                                  |
    \*-----------------------------------------------------*/
    DetectDevicesMutex.lock();

    /*-----------------------------------------------------*\
    | Set detection in progress flag                        |
    \*-----------------------------------------------------*/
    detection_in_progress = true;

    /*-----------------------------------------------------*\
    | Initialize local variables                            |
    \*-----------------------------------------------------*/

    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|               Start device detection               |");
    LOG_INFO("------------------------------------------------------");

    /*-----------------------------------------------------*\
    | Start at 0% detection progress                        |
    \*-----------------------------------------------------*/
    detection_percent   = 0;
    detection_string    = "";

    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_DETECTION_PROGRESS_CHANGED);

    /*-----------------------------------------------------*\
    | Open device disable list and read in disabled         |
    | device strings                                        |
    \*-----------------------------------------------------*/
    json detector_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("Detectors");
    /*-----------------------------------------------------*\
    | Check initial detection delay setting                 |
    \*-----------------------------------------------------*/
    unsigned int initial_detection_delay_ms  = 0;
    if(detector_settings.contains("initial_detection_delay_ms"))
    {
        initial_detection_delay_ms = detector_settings["initial_detection_delay_ms"];
    }

    /*-----------------------------------------------------*\
    | If configured, delay detection for the configured     |
    | time only on first detection                          |
    \*-----------------------------------------------------*/
    if(initial_detection)
    {
        if(initial_detection_delay_ms != 0)
        {
            detection_string = "Waiting for detection delay";
            SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_DETECTION_PROGRESS_CHANGED);

            LOG_INFO("[%s] Delaying detection for %d ms", DETECTIONMANAGER, initial_detection_delay_ms);
            std::this_thread::sleep_for(initial_detection_delay_ms * 1ms);
        }

        initial_detection = false;
    }

    /*-----------------------------------------------------*\
    | Reset the size entry used flags vector                |
    \*-----------------------------------------------------*/
    detection_size_entry_used.resize(rgb_controllers_sizes.size());

    for(std::size_t size_idx = 0; size_idx < detection_size_entry_used.size(); size_idx++)
    {
        detection_size_entry_used[size_idx] = false;
    }

    detection_percent_denominator = i2c_group.GetNumDetectors()
                                    + hid_group.GetNumDetectors()
                                    + others_group.GetNumDetectors();
    /*------------------------------------------------------*\
    | Run groups detectors in order                          |
    | These internally register RGB devices as they're found |
    | Theoretically we can run these in parallel             |
    \*------------------------------------------------------*/
    i2c_group.Prepare();
    i2c_group.RunDetection();

    hid_group.Prepare();
    hid_group.RunDetection();

    others_group.Prepare();
    others_group.RunDetection();

    /*-----------------------------------------------------*\
    | Signal that detection progress reached 100%           |
    \*-----------------------------------------------------*/
    detection_percent     = 100;
    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_DETECTION_PROGRESS_CHANGED);

    /*-----------------------------------------------------*\
    | Signal that detection is complete                     |
    \*-----------------------------------------------------*/
    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_DETECTION_COMPLETE);

    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|                Detection completed                 |");
    LOG_INFO("------------------------------------------------------");

    /*-----------------------------------------------------*\
    | Clear detection in progress flag                      |
    \*-----------------------------------------------------*/
    detection_in_progress = false;

    /*-----------------------------------------------------*\
    | Unlock detection mutex                                |
    \*-----------------------------------------------------*/
    DetectDevicesMutex.unlock();

#ifdef __linux__
    /*-----------------------------------------------------*\
    | Check if the udev rules exist                         |
    \*-----------------------------------------------------*/
    bool udev_not_exist     = false;
    bool udev_multiple      = false;

    if(access("/etc/udev/rules.d/60-openrgb.rules", F_OK) != 0)
    {
        if(access("/usr/lib/udev/rules.d/60-openrgb.rules", F_OK) != 0)
        {
            udev_not_exist  = true;
        }
    }
    else
    {
        if(access("/usr/lib/udev/rules.d/60-openrgb.rules", F_OK) == 0)
        {
            udev_multiple   = true;
        }
    }

    /*-----------------------------------------------------*\
    | If the udev rules file is not installed, show a dialog|
    \*-----------------------------------------------------*/
    if(udev_not_exist)
    {
        LOG_DIALOG("%s", UDEV_MISSING);

        udev_multiple       = false;
    }

    /*-----------------------------------------------------*\
    | If multiple udev rules files are installed, show a    |
    | dialog                                                |
    \*-----------------------------------------------------*/
    else if(udev_multiple)
    {
        LOG_DIALOG("%s", UDEV_MUTLI);
    }

    /*-----------------------------------------------------*\
    | If any i2c interfaces failed to detect due to an      |
    | error condition, show a dialog                        |
    \*-----------------------------------------------------*/
    else if (i2c_group.I2CInterfaceFailure())
    {
        LOG_DIALOG("%s", I2C_ERR_LINUX);
    }

#endif

#ifdef _WIN32
    /*-----------------------------------------------------*\
    | If any i2c interfaces failed to detect due to an      |
    | error condition, show a dialog                        |
    \*-----------------------------------------------------*/
    if(i2c_group.I2CInterfaceFailure())
    {
        LOG_DIALOG("%s", I2C_ERR_WIN);
    }
#endif
}

/*---------------------------------------------------------*\
| Detection processing functions                            |
\*---------------------------------------------------------*/
void DetectionManager::ProcessCleanup()
{
    WaitForDetection();

    /*--------------------------*\
    | Clean up detectors          |
    \*--------------------------*/
    i2c_group.CleanUp();
    hid_group.CleanUp();
    others_group.CleanUp();

    /*-----------------------------------------------------*\
    | Make a copy of the list so that the controllers can   |
    | be deleted after the list is cleared                  |
    \*-----------------------------------------------------*/
    std::vector<RGBController *> rgb_controllers_copy = rgb_controllers;

    /*-----------------------------------------------------*\
    | Clear the controllers list                            |
    \*-----------------------------------------------------*/
    rgb_controllers.clear();

    /*-----------------------------------------------------*\
    | Signal the list cleared callback                      |
    \*-----------------------------------------------------*/
    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_RGBCONTROLLER_LIST_CLEARED);

    /*-----------------------------------------------------*\
    | Delete the controllers                                |
    \*-----------------------------------------------------*/
    for(RGBController* rgb_controller : rgb_controllers_copy)
    {
        delete rgb_controller;
    }
}

void DetectionManager::ProcessDynamicDetectors()
{
    for(std::size_t detector_idx = 0; detector_idx < dynamic_detectors.size(); detector_idx++)
    {
        LOG_INFO("Running dynamic detector %s", dynamic_detectors[detector_idx].name.c_str());
        dynamic_detectors[detector_idx].detector();
    }

    dynamic_detectors_processed = true;
}

bool DetectionManager::ProcessPreDetection()
{
    /*-----------------------------------------------------*\
    | Check if detection is already in progress before      |
    | continuing                                            |
    \*-----------------------------------------------------*/
    if(detection_in_progress.load())
    {
        return false;
    }

    /*-----------------------------------------------------*\
    | Process pre-detection hooks                           |
    \*-----------------------------------------------------*/
    ProcessPreDetectionHooks();

    /*-----------------------------------------------------*\
    | Process Dynamic Detectors                             |
    \*-----------------------------------------------------*/
    if(!dynamic_detectors_processed)
    {
        ProcessDynamicDetectors();
    }

    /*-----------------------------------------------------*\
    | Call detection start callbacks                        |
    \*-----------------------------------------------------*/
    LOG_TRACE("[%s] Signaling detection start", DETECTIONMANAGER);

    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_DETECTION_STARTED);

    /*-----------------------------------------------------*\
    | Update the detector settings                          |
    \*-----------------------------------------------------*/
    UpdateDetectorSettings();

    /*-----------------------------------------------------*\
    | Initialize sizes list                                 |
    \*-----------------------------------------------------*/
    LOG_DEBUG("get controller size");

    rgb_controllers_sizes = ResourceManager::get()->GetProfileManager()->GetControllerListFromSizes();

    /*-----------------------------------------------------*\
    | Clean up any existing detected devices                |
    \*-----------------------------------------------------*/
    LOG_DEBUG("process cleanup");
    ProcessCleanup();

    return true;
}

void DetectionManager::ProcessPreDetectionHooks()
{
    for(std::size_t hook_idx = 0; hook_idx < pre_detection_hooks.size(); hook_idx++)
    {
        pre_detection_hooks[hook_idx]();
    }
}

void DetectionManager::UpdateDetectorSettings()
{
    /*-----------------------------------------------------*\
    | Open device disable list and read in disabled device  |
    | strings                                               |
    \*-----------------------------------------------------*/
    json detector_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("Detectors");
    bool save_settings = false;

    std::vector<const std::string*> detector_names;

    std::vector<const std::string*> i2c_names = i2c_group.GetDetectorNames();
    std::vector<const std::string*> hid_names = hid_group.GetDetectorNames();
    std::vector<const std::string*> others_names = others_group.GetDetectorNames();

    detector_names.insert(detector_names.end(), i2c_names.begin(), i2c_names.end());
    detector_names.insert(detector_names.end(), hid_names.begin(), hid_names.end());
    detector_names.insert(detector_names.end(), others_names.begin(), others_names.end());

    for (const std::string* detector_name : detector_names)
    {
        if(!(detector_settings.contains("detectors") && detector_settings["detectors"].contains(*detector_name)))
        {
            detector_settings["detectors"][*detector_name] = true;
            save_settings = true;
        }
    }

    /*-----------------------------------------------------*\
    | If there were any setting changes that need to be     |
    | saved, set the settings in the settings manager and   |
    | save them.                                            |
    \*-----------------------------------------------------*/
    if(save_settings)
    {
        LOG_INFO("[%s] Saving detector settings", DETECTIONMANAGER);
        ResourceManager::get()->GetSettingsManager()->SetSettings("Detectors", detector_settings);
        ResourceManager::get()->GetSettingsManager()->SaveSettings();
    }

    /*--------------------------------*\
    | apply updates to detector groups |
    \*--------------------------------*/
    i2c_group.UpdateSettings();
    hid_group.UpdateSettings();
    others_group.UpdateSettings();
}

/*---------------------------------------------------------*\
| Function for signalling DetectionManager updates to       |
| registered callbacks                                      |
\*---------------------------------------------------------*/
void DetectionManager::SignalUpdate(unsigned int update_reason)
{
    DetectionCallbackMutex.lock();

    for(std::size_t callback_idx = 0; callback_idx < DetectionCallbacks.size(); callback_idx++)
    {
        DetectionCallbacks[callback_idx](DetectionCallbackArgs[callback_idx], update_reason);
    }

    DetectionCallbackMutex.unlock();

    LOG_TRACE("[%s] Update signalled: %d.", DETECTIONMANAGER, update_reason);
}

void DetectionManager::IncrementDetectorCount()
{
    detection_percent_numerator++;
    detection_percent = (unsigned int)(100.0f * (detection_percent_numerator / detection_percent_denominator));
}

void DetectionManager::SetDetectorString(std::string _detection_string)
{
    detection_string = _detection_string.c_str();
}
