/*---------------------------------------------------------*\
| ColorfulARGBController.cpp                                |
|                                                             |
|   PROTOCOL (reverse-engineered from USB capture and a     |
|   byte-by-byte diff against the official iGame Center):    |
|                                                             |
|   64-byte HID report "on the wire" (the device does not    |
|   use numbered reports -- Report ID = 0):                  |
|     byte[0]      = 0x01                                    |
|     byte[1]      = 0x00                                    |
|     byte[2]      = 0x88               (command, always the |
|                     same)                                   |
|     byte[3]      = seq -- an 11-step cycle: 0x00..0x09     |
|                     then 0xFF, then back to 0x00. The      |
|                     seq=0xFF packet MUST have zeroed RGB -- |
|                     it is the commit/sync signal that makes |
|                     the device actually apply the batched   |
|                     colors to the physical LEDs. Without    |
|                     this packet, the color never changes    |
|                     even though every write() succeeds.     |
|     byte[4..64]   = 20 RGB triplets (60 bytes), one per     |
|                     LED "zone".                              |
|                                                             |
|   CROSS-PLATFORM NOTE (hidapi):                             |
|   The Windows backend (hidapi_winapi) requires 1 EXTRA      |
|   dummy byte prepended to the buffer (on top of the real    |
|   64-byte report) before calling hid_write(), otherwise     |
|   the first byte of the report gets truncated by Windows'   |
|   HID class driver -- this was validated via USB capture    |
|   during protocol research (see the standalone Rust         |
|   project). The Linux backend (hidraw) does NOT need the    |
|   extra dummy byte -- the 64-byte buffer is sent as-is to   |
|   /dev/hidrawX. That's why SendPacket() below differs in    |
|   buffer size/offset between _WIN32 and other platforms.    |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-only                    |
\*---------------------------------------------------------*/

#include "ColorfulARGBController.h"
#include <cstring>

ColorfulARGBController::ColorfulARGBController(hid_device* dev_handle, const char* path)
{
    dev      = dev_handle;
    location = path;
}

ColorfulARGBController::~ColorfulARGBController()
{
    hid_close(dev);
}

std::string ColorfulARGBController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string ColorfulARGBController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    std::wstring return_wstring = serial_string;
    return(std::string(return_wstring.begin(), return_wstring.end()));
}

void ColorfulARGBController::SendPacket(unsigned char* zone_rgb, unsigned char seq)
{
#ifdef _WIN32
    /*-------------------------------------------------------------*\
    | Windows: prepend 1 dummy 0x00 byte (not part of the real     |
    | 64-byte report) so hidapi_winapi doesn't truncate the first  |
    | byte of the actual data.                                     |
    \*-------------------------------------------------------------*/
    unsigned char usb_buf[COLORFUL_ARGB_REPORT_LEN + 1];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /* usb_buf[0] = dummy, leave it 0x00 */
    usb_buf[1] = 0x01;
    usb_buf[2] = 0x00;
    usb_buf[3] = 0x88;
    usb_buf[4] = seq;

    /* Sync packet (seq=0xFF) always has zeroed RGB -- don't fill zone_rgb */
    if((seq != 0xFF) && (zone_rgb != nullptr))
    {
        memcpy(&usb_buf[5], zone_rgb, COLORFUL_ARGB_NUM_ZONES * 3);
    }

    hid_write(dev, usb_buf, sizeof(usb_buf));
#else
    /*-------------------------------------------------------------*\
    | Linux (hidraw): send exactly 64 bytes, no extra dummy byte -- |
    | identical to the original USB capture.                        |
    \*-------------------------------------------------------------*/
    unsigned char usb_buf[COLORFUL_ARGB_REPORT_LEN];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0] = 0x01;
    usb_buf[1] = 0x00;
    usb_buf[2] = 0x88;
    usb_buf[3] = seq;

    /* Sync packet (seq=0xFF) always has zeroed RGB -- don't fill zone_rgb */
    if((seq != 0xFF) && (zone_rgb != nullptr))
    {
        memcpy(&usb_buf[4], zone_rgb, COLORFUL_ARGB_NUM_ZONES * 3);
    }

    hid_write(dev, usb_buf, sizeof(usb_buf));
#endif
}

void ColorfulARGBController::SetColors(unsigned char* colors_rgb, unsigned int num_zones)
{
    unsigned char zone_buf[COLORFUL_ARGB_NUM_ZONES * 3];
    memset(zone_buf, 0x00, sizeof(zone_buf));

    unsigned int copy_zones = (num_zones < COLORFUL_ARGB_NUM_ZONES) ? num_zones : COLORFUL_ARGB_NUM_ZONES;
    memcpy(zone_buf, colors_rgb, copy_zones * 3);

    /* Full cycle: 10 data packets (seq 0x00-0x09) + 1 commit/sync packet (seq 0xFF) */
    for(unsigned char seq = 0x00; seq <= 0x09; seq++)
    {
        SendPacket(zone_buf, seq);
    }

    SendPacket(nullptr, 0xFF);
}
