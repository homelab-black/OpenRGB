#include<map>
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

std::vector<std::array<unsigned char, 14>> GenericWirelessKeyboardController::SplitChunks(const std::vector<unsigned char>& data)
{
    const size_t max_chunk_size = 14;
    std::vector<std::array<unsigned char, 14>> result;
    size_t offset = 0;

    while (offset < data.size()) {
        std::array<unsigned char, 14> chunk = {};
        size_t end = std::min(max_chunk_size, data.size() - offset);
        std::copy_n(data.begin() + offset, end, chunk.begin());
        result.push_back(chunk);
        offset += end;
    }

    return result;
}



void GenericWirelessKeyboardController::SetSolidColor(const RGBColor &color)
{
    unsigned char packet[20] = {0x13, 0x88, 0x01, 0x00, 0x23};

    packet[5] = RGBGetRValue(color);
    packet[6] = RGBGetGValue(color);
    packet[7] = RGBGetBValue(color);

    packet[19] = CalculateCrc(packet);

    int res = hid_write(dev, packet, sizeof(packet));
    if (res < 0) {
        LOG_ERROR("Unable to write to Generic Wireless Keyboard.");
    }
}

void GenericWirelessKeyboardController::SetLEDColors(std::vector<led>& leds, std::vector<RGBColor>& colors)
{
    if (std::adjacent_find(colors.begin(), colors.end(), std::not_equal_to<>()) == colors.end()) {
        SetSolidColor(colors[0]);
        return;
    }

    std::map<RGBColor, std::vector<unsigned int>> color_map;
    for (size_t i = 0; i < leds.size(); i++)
    {
        RGBColor color = colors[i];

        if(color == ToRGBColor(0, 0, 0))
        {
            continue;
        }

        color_map[color].push_back(leds[i].value);
    }

    std::vector<unsigned char> payload;

    for(std::map<RGBColor, std::vector<unsigned int> >::const_iterator it = color_map.begin(); it != color_map.end(); ++it)
    {
        payload.push_back(RGBGetRValue(it->first));
        payload.push_back(RGBGetGValue(it->first));
        payload.push_back(RGBGetBValue(it->first));
        payload.push_back(it->second.size());

        for(size_t i = 0; i < it->second.size(); i++)
        {
            payload.push_back(it->second[i]);
        }

    }

    std::vector<std::array<unsigned char, 14>> chunks = SplitChunks(payload);

    for(size_t i = 0; i < chunks.size(); i++)
    {
        unsigned char packet[20] = {0x13, 0x88, 0x01, 0x00, 0x1e};
        packet[2] = chunks.size();
        packet[3] = i;

        for(size_t j = 0; j < 14; j++)
        {
            packet[5 + j] = chunks[i][j];
        }

        packet[19] = CalculateCrc(packet);

        int res = hid_write(dev, packet, sizeof(packet));

        if (res < 0) {
            LOG_ERROR("Unable to write to Generic Wireless Keyboard.");
        }
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
