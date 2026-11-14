#include "GenericWirelessKeyboardController.h"
#include "LogManager.h"

GenericWirelessKeyboardController::GenericWirelessKeyboardController(hid_device* dev_handle, const std::string& path, const std::string& dev_name)
{
    dev = dev_handle;
    location = path;
    name = dev_name;
}

GenericWirelessKeyboardController::~GenericWirelessKeyboardController()
{
    if (dev) {
        hid_close(dev);
        dev = nullptr;
    }
}

void GenericWirelessKeyboardController::SendColors(const std::vector<RGBColor>& colors)
{
    unsigned char packet[20] = { 0x13, 0x88, 0x01, 0x00, 0x23 };

    if (colors.size() > 0)
    {
        packet[5] = RGBGetRValue(colors[0]);
        packet[6] = RGBGetGValue(colors[0]);
        packet[7] = RGBGetBValue(colors[0]);
    }

    packet[19] = CalculateCrc(packet);

    int res = hid_write(dev, packet, sizeof(packet));
    if (res < 0) {
        LOG_ERROR("Unable to write to Generic Wireless Keyboard.");
    }
}

std::string GenericWirelessKeyboardController::GetName()
{
    return name;
}

std::string GenericWirelessKeyboardController::GetLocation()
{
    return location;
}

uint8_t GenericWirelessKeyboardController::CalculateCrc(const unsigned char *report)
{
    uint8_t crc = 0;
    size_t len = sizeof(report) - 1;

    for (size_t i = 0; i < len; i++)
    {
        crc += report[i];
    }
    return crc & 0xFF;
}
