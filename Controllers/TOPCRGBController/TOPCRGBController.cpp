#include <cstring>
#include <iomanip>
#include <sstream>
#include "TOPCRGBController.h"
#include "LogManager.h"

/*---------------------------------------------------------*\
| Low level protocol                                        |
|                                                           |
| Taken verbatim from the vendor reference implementation    |
| (backend/effects/static.py and friends).  Every effect     |
| builds the same 256 byte feature report and sends it four  |
| times, once for each report ID 0x10, 0x11, 0x12 and 0x13,  |
| followed by an apply report after each:                    |
|                                                           |
|   colour report                                            |
|     byte 0        0x10 + channel (0x10 .. 0x13)           |
|     byte 1        0x00                                     |
|     byte 2        0x00                                     |
|     byte 3..182   60 * (R, G, B)                          |
|     byte 183..255 padding zeros                            |
|                                                           |
|   apply report                                             |
|     byte 0        0x01                                     |
|     byte 1        0x00                                     |
|     byte 2        0x88                                     |
|     byte 3        0xFF                                     |
|     byte 4..255   padding zeros                            |
|                                                           |
| There is no LED count parameter in the reference code; it  |
| always fills 60 LEDs and mirrors the same colours to all   |
| four report IDs.                                           |
\*---------------------------------------------------------*/

const char* TOPC_RGB_DRIVER_NAME = "TOPCRGBController";

TOPCRGBController::TOPCRGBController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;

    LOG_DEBUG("[%s] Constructed for device at %s", TOPC_RGB_DRIVER_NAME, location.c_str());
    LOG_DEBUG("[%s] Config: %u report IDs (0x%02X..0x%02X), %u LEDs per report, report size %u",
              TOPC_RGB_DRIVER_NAME,
              (unsigned int)TOPC_RGB_REPORT_ID_COUNT,
              (unsigned int)TOPC_RGB_REPORT_ID_BASE,
              (unsigned int)(TOPC_RGB_REPORT_ID_BASE + TOPC_RGB_REPORT_ID_COUNT - 1),
              (unsigned int)TOPC_RGB_LEDS_PER_REPORT,
              (unsigned int)TOPC_RGB_REPORT_SIZE);
}

TOPCRGBController::~TOPCRGBController()
{
    LOG_DEBUG("[%s] Destructor called for %s", TOPC_RGB_DRIVER_NAME, location.c_str());

    if(dev != nullptr)
    {
        hid_close(dev);
        dev = nullptr;
    }
}

std::string TOPCRGBController::GetDeviceLocation()
{
    return("HID: " + location);
}

void TOPCRGBController::SendFeatureReport(unsigned char* report, int size, const char* what)
{
    if(dev == nullptr)
    {
        LOG_ERROR("[%s] Attempted to send %s report with a NULL device handle",
                  TOPC_RGB_DRIVER_NAME, what);
        return;
    }

#if TOPC_RGB_DEBUG_LOG
    /*-----------------------------------------------------*\
    | Build a compact hex dump of the report header so the   |
    | log shows what is actually being transmitted.          |
    \*-----------------------------------------------------*/
    std::ostringstream dump;

    dump << std::hex << std::setfill('0');

    for(int idx = 0; idx < 16 && idx < size; idx++)
    {
        dump << std::setw(2) << (unsigned int)report[idx];

        if(idx < 15)
        {
            dump << " ";
        }
    }

    LOG_DEBUG("[%s] Sending %s report (size %d): %s ...",
              TOPC_RGB_DRIVER_NAME, what, size, dump.str().c_str());
#endif

    int res = hid_send_feature_report(dev, report, size);

    if(res < 0)
    {
        LOG_ERROR("[%s] hid_send_feature_report failed for %s report (returned %d)",
                  TOPC_RGB_DRIVER_NAME, what, res);
    }
    else if(res != size)
    {
        LOG_WARNING("[%s] hid_send_feature_report sent %d of %d bytes for %s report",
                    TOPC_RGB_DRIVER_NAME, res, size, what);
    }
#if TOPC_RGB_DEBUG_LOG
    else
    {
        LOG_DEBUG("[%s] %s report sent successfully (%d bytes)",
                  TOPC_RGB_DRIVER_NAME, what, res);
    }
#endif
}

void TOPCRGBController::ApplyReport()
{
    unsigned char apply[TOPC_RGB_REPORT_SIZE];

    memset(apply, 0x00, sizeof(apply));

    apply[0] = TOPC_RGB_APPLY_REPORT_ID;
    apply[1] = 0x00;
    apply[2] = 0x88;
    apply[3] = 0xFF;

    SendFeatureReport(apply, sizeof(apply), "apply");
}

void TOPCRGBController::SetLEDs(RGBColor* colors, unsigned int num_colors)
{
    unsigned char report[TOPC_RGB_REPORT_SIZE];

    /*-----------------------------------------------------*\
    | Build the shared colour payload once, exactly like the |
    | reference implementation:                              |
    |   [report_id, 0x00, 0x00] + [R,G,B] * 60 + padding     |
    | If fewer than 60 colours are supplied, repeat the last |
    | one to fill the report.                                |
    \*-----------------------------------------------------*/
    memset(report, 0x00, sizeof(report));

    report[0]   = TOPC_RGB_REPORT_ID_BASE;
    report[1]   = 0x00;
    report[2]   = 0x00;

    unsigned int    color_count = num_colors;
    RGBColor        last_color  = (color_count > 0) ? colors[color_count - 1] : 0;

    if(color_count > TOPC_RGB_LEDS_PER_REPORT)
    {
        color_count = TOPC_RGB_LEDS_PER_REPORT;
    }

    for(unsigned int led = 0; led < TOPC_RGB_LEDS_PER_REPORT; led++)
    {
        RGBColor color = (led < color_count) ? colors[led] : last_color;

        report[3 + (led * TOPC_RGB_COLOR_DEPTH) + 0] = RGBGetRValue(color);
        report[3 + (led * TOPC_RGB_COLOR_DEPTH) + 1] = RGBGetGValue(color);
        report[3 + (led * TOPC_RGB_COLOR_DEPTH) + 2] = RGBGetBValue(color);
    }

    LOG_DEBUG("[%s] SetLEDs: %u colour(s) supplied, first colour %02X%02X%02X",
              TOPC_RGB_DRIVER_NAME,
              num_colors,
              (unsigned int)report[3],
              (unsigned int)report[4],
              (unsigned int)report[5]);

    std::lock_guard<std::mutex> lock(dev_mutex);

    /*-----------------------------------------------------*\
    | Send the colour report for every report ID, applying   |
    | after each one, exactly like the reference code.       |
    \*-----------------------------------------------------*/
    for(unsigned int channel = 0; channel < TOPC_RGB_REPORT_ID_COUNT; channel++)
    {
        report[0] = TOPC_RGB_REPORT_ID_BASE + channel;

        LOG_DEBUG("[%s]   colour report ID 0x%02X", TOPC_RGB_DRIVER_NAME, (unsigned int)report[0]);

        SendFeatureReport(report, sizeof(report), "colour");
        ApplyReport();
    }
}
