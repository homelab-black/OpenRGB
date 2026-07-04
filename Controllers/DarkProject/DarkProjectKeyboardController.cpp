/*---------------------------------------------------------*\
| DarkProjectKeyboardController.cpp                         |
|                                                           |
|   Driver for Dark Project keyboard                        |
|                                                           |
|   Chris M (DrNo)                              08 Apr 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DarkProjectKeyboardController.h"
#include "LogManager.h"
#include "StringUtils.h"
#include <algorithm>

static uint8_t packet_map[88] =
{
    /*00        ESC  F1   F2   F3   F4   F5   F6   F7   F8   F9  */
                 5,  11,  17,  23,  29,  35,  41,  47,  53,  59,

    /*10        F10  F11  F12  PRT  SLK  PBK   `    1    2    3  */
                65,  71,  77,  83,  89,  95,   0,   6,  12,  18,

    /*20         4    5    6    7    8    9    0    -    =   BSP */
                24,  30,  36,  42,  48,  54,  60,  66,  72,  78,

    /*30        INS  HME  PUP  TAB   Q    W    E    R    T    Y  */
                84,  90,  96,   1,   7,  13,  19,  25,  31,  37,

    /*40         U    I    O    P    [    ]    \   DEL  END  PDN */
                 43,  49,  55,  61,  67,  73,  79,  85,  91,  97,

    /*50        CAP   A    S    D    F    G    H    J    K    L  */
                 2,   8,  14,  20,  26,  32,  38,  44,  50,  56,

    /*60         ;    '   ENT  LSH   Z    X    C    V    B    N  */
                 62,  68,  80,   3,  15,  21,  27,  33,  39,  45,

    /*70         M    ,    .    /   RSH  UP  LCTL LWIN LALT SPC  */
                 51,  57,  63,  69,  81,  93,   4,  10,  16,  34,

    /*80       RALT RFNC MENU RCTL  LFT  DWN  RGT                */
                52,  58,  64,  76,  88,  94, 100

                                                                                                                  /* Missing Indexes 9, 22, 28, 40, 46, 70, 74, 75, 82, 86, 87, 92, 98, 99, 101 */
};

DarkProjectKeyboardController::DarkProjectKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev = dev_handle;
    location = path;
    name = dev_name;
}

DarkProjectKeyboardController::~DarkProjectKeyboardController()
{
    hid_close(dev);
}

std::string DarkProjectKeyboardController::GetLocation()
{
    return("HID: " + location);
}

std::string DarkProjectKeyboardController::GetName()
{
    return(name);
}

std::string DarkProjectKeyboardController::GetSerial()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if (ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void DarkProjectKeyboardController::SetLedsDirect(std::vector<RGBColor> colors)
{
    uint8_t RGbuffer[DARKPROJECTKEYBOARD_PACKET_SIZE] = { 0x08, 0x07, 0x00, 0x00, 0x00 };
    uint8_t BAbuffer[DARKPROJECTKEYBOARD_PACKET_SIZE] = { 0x08, 0x07, 0x00, 0x01, 0x00 };

    size_t map_len = sizeof(packet_map) / sizeof(packet_map[0]);
    size_t count = std::min(colors.size(), map_len);

    for (size_t i = 0; i < count; i++)
    {
        RGBColor key = colors[i];
        uint16_t offset = packet_map[i];

        size_t r_index = DARKPROJECTKEYBOARD_RED_BLUE_BYTE + offset;
        size_t g_index = DARKPROJECTKEYBOARD_GREEN_BYTE + offset;

        // Ensure we don't write past the packet buffer
        if (r_index >= DARKPROJECTKEYBOARD_PACKET_SIZE || g_index >= DARKPROJECTKEYBOARD_PACKET_SIZE)
            continue;

        RGbuffer[r_index] = RGBGetRValue(key);
        RGbuffer[g_index] = RGBGetGValue(key);
        BAbuffer[r_index] = RGBGetBValue(key);
    }

    int ret = hid_write(dev, RGbuffer, DARKPROJECTKEYBOARD_PACKET_SIZE);
    if (ret < 0) {
        // write failed - consider logging if LogManager API is available
    }
    ret = hid_write(dev, BAbuffer, DARKPROJECTKEYBOARD_PACKET_SIZE);
    if (ret < 0) {
        // write failed - consider logging if LogManager API is available
    }
}

#if 0

// ========== НОВЫЙ КОД ДЛЯ КЛАВИАТУРЫ KD87A==========
/*---------------------------------------------------------*\
|  Dark Project ONE KD87A                                   |
\*---------------------------------------------------------*/
#define DARKPROJECT_KD87A_VID                              0x1A2C
#define DARKPROJECT_KD87A_PID                              0x1514

void DetectDarkProjectKD87AControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);
    if (dev)
    {
        DarkProjectKeyboardController* controller = new DarkProjectKeyboardController(dev, info->path, name);
        RGBController_DarkProjectKeyboard* rgb_controller = new RGBController_DarkProjectKeyboard(controller);
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}
REGISTER_HID_DETECTOR("Dark Project KD87A Skeleton", DetectDarkProjectKD87AControllers, DARKPROJECT_KD87A_VID, DARKPROJECT_KD87A_PID);
#endif
