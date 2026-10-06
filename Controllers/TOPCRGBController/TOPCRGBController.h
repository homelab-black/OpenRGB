#pragma once

#include <string>
#include <mutex>
#include <hidapi.h>
#include "RGBController.h"

#define TOPC_RGB_VID                    0x8888
#define TOPC_RGB_PID                    0x7A95

/*---------------------------------------------------------*\
| HID usage page/usage used to uniquely identify the RGB    |
| control interface.  Taken from the device HardwareID:     |
|   HID\VID_8888&PID_7A95&UP:FF01_U:0001                    |
\*---------------------------------------------------------*/
#define TOPC_RGB_USAGE_PAGE             0xFF01
#define TOPC_RGB_USAGE                  0x0001

/*---------------------------------------------------------*\
| Protocol constants, taken verbatim from the vendor        |
| reference implementation in backend/effects/static.py.    |
|                                                           |
| The device is a single 3-pin addressable RGB header.  The |
| firmware is driven by writing the SAME colour data to      |
| four feature reports (IDs 0x10, 0x11, 0x12, 0x13), each    |
| followed by an apply report.  The reference code does not |
| expose any LED count; it always fills 60 LEDs per report. |
\*---------------------------------------------------------*/
#define TOPC_RGB_REPORT_ID_BASE         0x10                /* First colour report ID           */
#define TOPC_RGB_REPORT_ID_COUNT        4                   /* Reports 0x10, 0x11, 0x12, 0x13   */
#define TOPC_RGB_LEDS_PER_REPORT        60                  /* LEDs filled in every report      */

/*---------------------------------------------------------*\
| TOPC_RGB_LED_COUNT is the default number of LEDs exposed   |
| to OpenRGB (the initial zone size).  The zone is user      |
| resizable in the UI between 1 and TOPC_RGB_LEDS_PER_REPORT |
| (60), so this is only the value used on first creation.    |
|                                                            |
| The reference code always fills 60 LED slots in the report |
| regardless of the zone size, so colours are padded up to   |
| TOPC_RGB_LEDS_PER_REPORT when fewer LEDs are configured.   |
\*---------------------------------------------------------*/
#define TOPC_RGB_LED_COUNT              60                  /* Default LEDs exposed to OpenRGB  */

#define TOPC_RGB_REPORT_SIZE            256                 /* HID feature report size          */
#define TOPC_RGB_COLOR_DEPTH            3                   /* Bytes per LED (R, G, B, no alpha)*/

/*---------------------------------------------------------*\
| Apply command, sent after each colour report              |
|   [0x01, 0x00, 0x88, 0xFF, 0, 0, ...]                      |
\*---------------------------------------------------------*/
#define TOPC_RGB_APPLY_REPORT_ID        0x01

/*---------------------------------------------------------*\
| Debug logging.  Set TOPC_RGB_DEBUG_LOG to 1 to emit       |
| verbose LOG_DEBUG trace of every report sent to the       |
| device (report ID + first few color bytes).  This shows   |
| up in the OpenRGB log when Log Level >= Debug is set in   |
| the Log Manager settings.                                 |
\*---------------------------------------------------------*/
#define TOPC_RGB_DEBUG_LOG              1

class TOPCRGBController
{
public:
    TOPCRGBController(hid_device* dev, const char* path);
    ~TOPCRGBController();

    std::string     GetDeviceLocation();

    /*-----------------------------------------------------*\
    | SetLEDs broadcasts one colour buffer to every report   |
    | (0x10..0x13), followed by an apply report after each,  |
    | exactly like the vendor reference implementation.      |
    \*-----------------------------------------------------*/
    void            SetLEDs(RGBColor* colors, unsigned int num_colors);
private:
    hid_device*     dev;
    std::string     location;
    std::mutex      dev_mutex;

    void            SendFeatureReport(unsigned char* report, int size, const char* what);
    void            ApplyReport();
};

