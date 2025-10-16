#include "SinowealthWirelessKeyboardController.h"
#include "LogManager.h"

using namespace std::chrono_literals;

SinowealthWirelessKeyboardController::SinowealthWirelessKeyboardController(hid_device*        dev_handle,
                                                                           const std::string& path,
                                                                           const std::string& dev_name)
{
    dev      = dev_handle;
    location = path;
    name     = dev_name;
}

SinowealthWirelessKeyboardController::~SinowealthWirelessKeyboardController()
{
    if(dev)
    {
        hid_close(dev);
        dev = nullptr;
    }
}

std::vector<std::vector<unsigned char>>
SinowealthWirelessKeyboardController::SplitChunks(const std::vector<unsigned char>& data)
{
    const size_t                            max_chunk_size = 14;
    std::vector<std::vector<unsigned char>> result;
    size_t                                  offset = 0;

    while(offset < data.size())
    {
        size_t                     end = std::min(max_chunk_size, data.size() - offset);
        std::vector<unsigned char> chunk(data.begin() + offset, data.begin() + offset + end);
        result.push_back(chunk);
        offset += end;
    }

    return result;
}

void SinowealthWirelessKeyboardController::SendSolidColor(const RGBColor& color)
{
    if(!lock.try_lock())
    {
        return;
    }

    unsigned char packet[20] = {0x13, 0x88, 0x01, 0x00, 0x23};

    packet[5] = RGBGetRValue(color);
    packet[6] = RGBGetGValue(color);
    packet[7] = RGBGetBValue(color);

    packet[19] = CalculateCrc(packet, 19);

    int res = hid_write(dev, packet, sizeof(packet));

    if(res < 0)
    {
        LOG_ERROR("Unable to write to Sinowealth Wireless Keyboard");
    }

    lock.unlock();
}

void SinowealthWirelessKeyboardController::SendLEDColors(
    std::unordered_map<RGBColor, std::vector<unsigned char>> led_color_map)
{
    if(!lock.try_lock())
    {
        return;
    }

    std::vector<unsigned char> payload;

    for(std::unordered_map<RGBColor, std::vector<unsigned char>>::const_iterator it = led_color_map.begin();
        it != led_color_map.end(); ++it)
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

    std::vector<std::vector<unsigned char>> chunks = SplitChunks(payload);

    for(size_t i = 0; i < chunks.size(); i++)
    {
        unsigned char packet[20] = {0x13, 0x88, 0x01, 0x00, 0x00};
        packet[2]                = chunks.size();
        packet[3]                = i;
        packet[4]                = chunks[i].size() + 0x10;

        for(size_t j = 0; j < 14; j++)
        {
            if(j < chunks[i].size())
                packet[5 + j] = chunks[i][j];
            else
                packet[5 + j] = 0x00;
        }

        packet[19] = CalculateCrc(packet, 19);

        int res = hid_write(dev, packet, sizeof(packet));

        if(res < 0)
        {
            LOG_ERROR("Unable to write to Sinowealth Wireless Keyboard");
        }

        LOG_ERROR("Written %d packet(s).", chunks.size());

        std::this_thread::sleep_for(5ms);
    }

    lock.unlock();
}

std::string SinowealthWirelessKeyboardController::GetName()
{
    return name;
}

std::string SinowealthWirelessKeyboardController::GetLocation()
{
    return location;
}

unsigned char SinowealthWirelessKeyboardController::CalculateCrc(const unsigned char* report, size_t len)
{
    unsigned char crc = 0;
    // Loop over the correct length (19 bytes)
    for(size_t i = 0; i < len - 1; i++)
    {
        crc += report[i];
    }
    return crc & 0xFF;
}
