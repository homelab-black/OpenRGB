/*-----------------------------------------*\
|  SteelSeriesApSController.cpp             |
|                                           |
|  Implementation for SteelSeries           |
|  ApS ALC and KLC controller               |
|                                           |
|  Morgan Guimard (morg) 1/25/2024          |
\*-----------------------------------------*/

#include "SteelSeriesApSController.h"
#include "LogManager.h"
#include <cstring>

using namespace std::chrono_literals;

SteelSeriesApSController::SteelSeriesApSController(hid_device* dev_handle, const hid_device_info& info)
{
    dev                 = dev_handle;
    location            = info.path;
    version             = "";

    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        serial_number = "";
    }
    else
    {
        std::wstring return_wstring = serial_string;
        serial_number = std::string(return_wstring.begin(), return_wstring.end());
    }
}

uint8_t SteelSeriesApSController::GetLayout()
{
    uint8_t query_buf[STEELSERIES_APS_WRITE_SIZE];
    memset(query_buf, 0x00, STEELSERIES_APS_WRITE_SIZE);
    query_buf[1] = 0x10;

    hid_write(dev, query_buf, STEELSERIES_APS_WRITE_SIZE);
    hid_read(dev, query_buf, STEELSERIES_APS_WRITE_SIZE);

    LOG_INFO("KLC DEBUG QUERY: 0x%02X, 0x%02X, 0x%02Xn", query_buf[0], query_buf[2], query_buf[2]);

    return query_buf[0];
}

SteelSeriesApSController::~SteelSeriesApSController()
{
    hid_close(dev);
}

std::string SteelSeriesApSController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string SteelSeriesApSController::GetSerialString()
{
    return(serial_number);
}

std::string SteelSeriesApSController::GetFirmwareVersion()
{
    return(version);
}

void SteelSeriesApSController::SetColors(const std::vector<RGBColor>& colors, bool use_provided_ids, const std::vector<unsigned int>& ids_orders)
{
    unsigned int max_leds_per_packet = 0x1E;
    unsigned int number_of_leds = colors.size();
    unsigned int total_leds_sent = 0;
    unsigned int led = 0;

    while(total_leds_sent < number_of_leds)
    {
        unsigned int remaining_leds = number_of_leds - total_leds_sent;
        unsigned int leds_in_packet = std::min<unsigned int>(remaining_leds, max_leds_per_packet);

        uint8_t report[STEELSERIES_APS_REPORT_SIZE];
        memset(report, 0x00, STEELSERIES_APS_REPORT_SIZE);

        report[1] = STEELSERIES_APS_REPORT_ID;
        report[3] = leds_in_packet;

        unsigned int offset = 5;

        for(unsigned int i = 0; i < leds_in_packet; i++)
        {
            report[offset]      = RGBGetRValue(colors[led]);
            report[offset + 1]  = RGBGetGValue(colors[led]);
            report[offset + 2]  = RGBGetBValue(colors[led]);
            report[offset + 9]  = 0x01;

            if(use_provided_ids)
            {
                report[offset + 11] = ids_orders[led];
            }
            else
            {
                report[offset + 11] = led;
            }

            offset += 12;
            led++;
        }

        hid_send_feature_report(dev, report, STEELSERIES_APS_REPORT_SIZE);
        total_leds_sent += leds_in_packet;
        //DebugPacket(report, STEELSERIES_APS_REPORT_SIZE);
        std::this_thread::sleep_for(10ms);
    }

    uint8_t apply_packet[STEELSERIES_APS_WRITE_SIZE];
    memset(apply_packet, 0x00, STEELSERIES_APS_WRITE_SIZE);
    apply_packet[1] = STEELSERIES_APS_WRITE_COMMAND;

    hid_write(dev, apply_packet, STEELSERIES_APS_WRITE_SIZE);

    std::this_thread::sleep_for(10ms);
}

void SteelSeriesApSController::DebugPacket(uint8_t *data, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++)
    {
        printf("%02X ", data[i]);
    }

    printf("\n");
}
