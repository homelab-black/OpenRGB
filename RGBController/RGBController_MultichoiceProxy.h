/*---------------------------------------------------------*\
| RGBController_MultichoiceProxy.h                          |
|                                                           |
|   Placeholder RGBController for devices with unresolvable |
|   detection conflicts, only creating the real             |
|   RGBController after receiving the user's consent        |
|                                                           |
|   Dmitry Kychanov (k1-801)                    07 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController_Proxy.h"

typedef std::vector<RGBController*> DetectedControllers;

struct ControllerSpawnOption
{
    std::string name;
    std::function<DetectedControllers(const std::string&)> spawn;
};
typedef std::vector<ControllerSpawnOption> ControllerSpawnOptions;

class RGBController_MultichoiceProxy : public RGBController_Proxy
{
public:
    RGBController_MultichoiceProxy(device_type placeholder_type, const std::string& placeholder_name, const ControllerSpawnOptions &options);
    ~RGBController_MultichoiceProxy();

    controller_flags GetFlags();
    void             DeviceUpdateDeviceSpecificConfiguration();
    void             RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg);
    void             UnregisterUpdateCallback(void * callback_arg);
    nlohmann::json   GetDeviceSpecificConfigurationSchema();
    nlohmann::json   GetDeviceSpecificConfiguration();
    void             SetDeviceSpecificConfiguration(nlohmann::json configuration_json);

private:
    ControllerSpawnOptions              options;
    int                                 chosen_option;

    std::mutex                          ProxyUpdateMutex;
    std::vector<RGBControllerCallback>  ProxyUpdateCallbacks;
    std::vector<void *>                 ProxyUpdateCallbackArgs;

    bool spawnController(int option);
    void InitDeviceSpecificConfiguration();
    void ApplyDeviceSpecificConfiguration();
};
