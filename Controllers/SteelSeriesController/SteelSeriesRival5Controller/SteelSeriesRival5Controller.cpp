/*---------------------------------------------------------*\
| SteelSeriesAerox5Controller.cpp                           |
|                                                           |
|   Driver for the Steelseries Aerox 5                      |
|                                                           |
|   Bobby Quantum (BobbyQuantum)                19 May 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "SteelSeriesRival5Controller.h"
#include "LogManager.h"
#include <cstring>

SteelSeriesRival5Controller::SteelSeriesRival5Controller(
    hid_device* dev_handle,
    steelseries_type proto_type,
    const char* path,
    std::string dev_name
)
    : SteelSeriesMouseController(
        dev_handle,
        proto_type,
        path,
        dev_name
    )


{
    memset(zone_colors, 0, sizeof(zone_colors));
}

SteelSeriesRival5Controller::~SteelSeriesRival5Controller()
{
    hid_close(dev);
}


steelseries_mouse SteelSeriesRival5Controller::GetMouse()
{
    return rival_5;
}

std::string SteelSeriesRival5Controller::GetFirmwareVersion()
{
    return "";
}



void SteelSeriesRival5Controller::SetLightEffectAll(uint8_t)
{
}


void SteelSeriesRival5Controller::Save()
{
}

void SteelSeriesRival5Controller::SetColor(
    unsigned char zone_id,
    unsigned char red,
    unsigned char green,
    unsigned char blue,
    unsigned char brightness
)
{
    (void)brightness;

    if(zone_id >= 10)
    {
        return;
    }

    zone_colors[zone_id][0] = red;
    zone_colors[zone_id][1] = green;
    zone_colors[zone_id][2] = blue;
}

void SteelSeriesRival5Controller::UpdateLEDs(
    const std::vector<RGBColor>& colors
)
{
    uint8_t report[64] = {0};

    report[0] = 0x00;
    report[1] = 0x21;
    report[2] = 0xFF;
    report[3] = 0x03;

    for(size_t i = 0; i < 10 && i < colors.size(); i++)
    {
        report[4 + (i * 3) + 0] = RGBGetRValue(colors[i]);
        report[4 + (i * 3) + 1] = RGBGetGValue(colors[i]);
        report[4 + (i * 3) + 2] = RGBGetBValue(colors[i]);
    }

    int result =
        hid_send_feature_report(
            dev,
            report,
            sizeof(report)
        );

    printf(
        "Rival5 feature report result: %d\n",
        result
    );
}
