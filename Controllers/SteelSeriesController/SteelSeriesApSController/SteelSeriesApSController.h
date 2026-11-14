/*-----------------------------------------*\
|  SteelSeriesApSController.h               |
|                                           |
|  Definitions and types for SteelSeries    |
|  ApS ALC and KLC controller               |
|                                           |
|  Morgan Guimard (morg) 1/25/2024          |
\*-----------------------------------------*/
#pragma once

#include "RGBController.h"
#include <string>
#include <vector>
#include <hidapi.h>

#define STEELSERIES_APS_REPORT_SIZE         525
#define STEELSERIES_APS_REPORT_ID           0x0E
#define STEELSERIES_APS_WRITE_SIZE          65
#define STEELSERIES_APS_WRITE_COMMAND       0x09

class SteelSeriesApSController
{
public:
    SteelSeriesApSController(hid_device* dev_handle, const hid_device_info& info);
    ~SteelSeriesApSController();
    uint8_t GetLayout();
    std::string GetSerialString();
    std::string GetDeviceLocation();
    std::string GetFirmwareVersion();
    void SetColors(const std::vector<RGBColor>& colors, bool use_provided_ids = false, const std::vector<unsigned int>& ids_orders = {});

private:
    hid_device* dev;
    std::string location;
    std::string serial_number;
    std::string version;

    void DebugPacket(uint8_t *data, unsigned int size);
};
