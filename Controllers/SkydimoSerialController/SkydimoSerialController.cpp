/*---------------------------------------------------------*\
| SkydimoSerialController.cpp                              |
|                                                           |
|   Driver for Skydimo serial devices                      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "SkydimoSerialController.h"

#include <algorithm>
#include <cctype>
#include <cstring>

#define SKYDIMO_SERIAL_BAUD        115200
#define SKYDIMO_MATRIX_NA          0xFFFFFFFF

enum class SkydimoLayoutKind
{
    None,
    Strip1,
    Perimeter3,
    Perimeter4,
    Sides2,
};

struct SkydimoModelEntry
{
    const char*         model;
    SkydimoLayoutKind   layout;
    unsigned int        zones[4];
    unsigned int        total;
    unsigned int        max_leds;
};

static const SkydimoModelEntry skydimo_model_entries[] =
{
    { "SK0121", SkydimoLayoutKind::Perimeter3, { 13, 25, 13, 0  }, 51,   0   },
    { "SK0124", SkydimoLayoutKind::Perimeter3, { 14, 26, 14, 0  }, 54,   0   },
    { "SK0127", SkydimoLayoutKind::Perimeter3, { 17, 31, 17, 0  }, 65,   0   },
    { "SK0132", SkydimoLayoutKind::Perimeter3, { 20, 37, 20, 0  }, 77,   0   },
    { "SK0134", SkydimoLayoutKind::Perimeter3, { 15, 41, 15, 0  }, 71,   0   },
    { "SK0149", SkydimoLayoutKind::Perimeter3, { 19, 69, 19, 0  }, 107,  0   },
    { "SK0201", SkydimoLayoutKind::Sides2,     { 20, 20, 0,  0  }, 40,   0   },
    { "SK0202", SkydimoLayoutKind::Sides2,     { 30, 30, 0,  0  }, 60,   0   },
    { "SK0204", SkydimoLayoutKind::Sides2,     { 25, 25, 0,  0  }, 50,   0   },
    { "SK0402", SkydimoLayoutKind::Strip1,     { 72, 0,  0,  0  }, 72,   0   },
    { "SK0403", SkydimoLayoutKind::Strip1,     { 96, 0,  0,  0  }, 96,   0   },
    { "SK0404", SkydimoLayoutKind::Strip1,     { 144, 0,  0,  0 }, 144,  0   },
    { "SK0410", SkydimoLayoutKind::None,       { 0,  0,  0,  0  }, 0,    300 },
    { "SK0801", SkydimoLayoutKind::Strip1,     { 2,  0,  0,  0  }, 2,    0   },
    { "SK0802", SkydimoLayoutKind::Strip1,     { 18, 0,  0,  0  }, 18,   0   },
    { "SK0803", SkydimoLayoutKind::Strip1,     { 10, 0,  0,  0  }, 10,   0   },
    { "SK0901", SkydimoLayoutKind::Strip1,     { 14, 0,  0,  0  }, 14,   0   },
    { "SK0E01", SkydimoLayoutKind::Strip1,     { 16, 0,  0,  0  }, 16,   0   },
    { "SK0F01", SkydimoLayoutKind::Sides2,     { 29, 29, 0,  0  }, 58,   0   },
    { "SK0F02", SkydimoLayoutKind::Sides2,     { 25, 25, 0,  0  }, 50,   0   },
    { "SK0H01", SkydimoLayoutKind::Strip1,     { 2,  0,  0,  0  }, 2,    0   },
    { "SK0H02", SkydimoLayoutKind::Strip1,     { 4,  0,  0,  0  }, 4,    0   },
    { "SK0K01", SkydimoLayoutKind::Strip1,     { 120, 0, 0,  0  }, 120,  0   },
    { "SK0K02", SkydimoLayoutKind::Strip1,     { 15, 0,  0,  0  }, 15,   0   },
    { "SK0L21", SkydimoLayoutKind::Perimeter4, { 13, 25, 13, 25 }, 76,   0   },
    { "SK0L24", SkydimoLayoutKind::Perimeter4, { 14, 26, 14, 26 }, 80,   0   },
    { "SK0L27", SkydimoLayoutKind::Perimeter4, { 17, 31, 17, 31 }, 96,   0   },
    { "SK0L32", SkydimoLayoutKind::Perimeter4, { 20, 37, 20, 37 }, 114,  0   },
    { "SK0L34", SkydimoLayoutKind::Perimeter4, { 15, 41, 15, 41 }, 112,  0   },
    { "SK0M01", SkydimoLayoutKind::Strip1,     { 24, 0,  0,  0  }, 24,   0   },
    { "SK0N01", SkydimoLayoutKind::Strip1,     { 256, 0, 0,  0  }, 256,  0   },
    { "SK0N02", SkydimoLayoutKind::Strip1,     { 1024, 0, 0, 0  }, 1024, 0   },
    { "SK0N03", SkydimoLayoutKind::Strip1,     { 253, 0, 0,  0  }, 253,  0   },
    { "SK0S01", SkydimoLayoutKind::Strip1,     { 32, 0,  0,  0  }, 32,   0   },
    { "SKA124", SkydimoLayoutKind::Perimeter3, { 18, 34, 18, 0  }, 70,   0   },
    { "SKA127", SkydimoLayoutKind::Perimeter3, { 20, 41, 20, 0  }, 81,   0   },
    { "SKA132", SkydimoLayoutKind::Perimeter3, { 25, 45, 25, 0  }, 95,   0   },
    { "SKA134", SkydimoLayoutKind::Perimeter3, { 21, 51, 21, 0  }, 93,   0   },
    { "SKB124", SkydimoLayoutKind::Perimeter4, { 18, 34, 18, 34 }, 104,  0   },
    { "SKB127", SkydimoLayoutKind::Perimeter4, { 20, 41, 20, 41 }, 122,  0   },
    { "SKB132", SkydimoLayoutKind::Perimeter4, { 25, 44, 25, 44 }, 138,  0   },
    { "SKB134", SkydimoLayoutKind::Perimeter4, { 21, 50, 21, 50 }, 142,  0   },
};

static std::string StringToUpper(const std::string& value)
{
    std::string result = value;

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch)
    {
        return static_cast<char>(std::toupper(ch));
    });

    return result;
}

static const SkydimoModelEntry* FindModelEntry(const std::string& model)
{
    for(const SkydimoModelEntry& entry: skydimo_model_entries)
    {
        if(model == entry.model)
        {
            return &entry;
        }
    }

    return nullptr;
}

static std::string BytesToHex(const std::vector<unsigned char>& bytes, std::size_t offset)
{
    static const char hex[] = "0123456789ABCDEF";
    std::string result;

    result.reserve((bytes.size() - offset) * 2);

    for(std::size_t idx = offset; idx < bytes.size(); idx++)
    {
        result.push_back(hex[(bytes[idx] >> 4) & 0x0F]);
        result.push_back(hex[bytes[idx] & 0x0F]);
    }

    return result;
}

SkydimoSerialController::SkydimoSerialController(const std::string& portname)
{
    port_name       = portname;
    model_name      = "Skydimo UNKNOWN";
    model_id        = "UNKNOWN";
    serial_id       = "000000";
    port_open       = serialport.serial_open(port_name.c_str(), SKYDIMO_SERIAL_BAUD);
    present         = false;
    resizable       = true;
    leds_count      = 1;
    leds_min        = 1;
    leds_max        = 150;
    matrix_width    = 0;
    matrix_height   = 0;
}

SkydimoSerialController::~SkydimoSerialController()
{

}

bool SkydimoSerialController::IsPresent()
{
    if(present)
    {
        return true;
    }

    present = QueryInfo();
    return present;
}

void SkydimoSerialController::DrainInput()
{
    char buffer[64];

    for(unsigned int attempt = 0; attempt < 8; attempt++)
    {
        if(serialport.serial_read(buffer, sizeof(buffer)) <= 0)
        {
            break;
        }
    }
}

bool SkydimoSerialController::QueryInfo()
{
    if(!port_open)
    {
        return false;
    }

    DrainInput();

    char query[] = "Moni-A";
    if(serialport.serial_write(query, 6) != 6)
    {
        return false;
    }

    std::vector<unsigned char> response;

    for(unsigned int attempt = 0; attempt < 10; attempt++)
    {
        char buffer[64];
        int bytes_read = serialport.serial_read(buffer, sizeof(buffer));

        if(bytes_read < 0)
        {
            return false;
        }

        if(bytes_read > 0)
        {
            response.insert(response.end(), buffer, buffer + bytes_read);
        }

        if(response.size() >= 63)
        {
            break;
        }

        if(!response.empty())
        {
            unsigned char last = response[response.size() - 1];
            unsigned char prev = response.size() >= 2 ? response[response.size() - 2] : 0;

            if((last == '\r') || (last == '\n') || (prev == '\r'))
            {
                break;
            }
        }
    }

    while(!response.empty() && ((response.back() == '\r') || (response.back() == '\n')))
    {
        response.pop_back();
    }

    if(response.size() < 6)
    {
        return false;
    }

    if((std::toupper(response[0]) != 'S') || (std::toupper(response[1]) != 'K'))
    {
        return false;
    }

    model_id = StringToUpper(std::string(response.begin(), response.begin() + 6));
    model_name = "Skydimo " + model_id;
    serial_id = "000000";

    std::vector<unsigned char>::iterator comma = std::find(response.begin(), response.end(), ',');
    if(comma != response.end())
    {
        std::size_t comma_offset = static_cast<std::size_t>(comma - response.begin());
        if(comma_offset < 6)
        {
            return false;
        }

        serial_id = BytesToHex(response, comma_offset + 1);
    }

    ConfigureOutput();
    return true;
}

void SkydimoSerialController::ConfigureOutput()
{
    const SkydimoModelEntry* entry = FindModelEntry(model_id);

    resizable       = true;
    leds_count      = 1;
    leds_min        = 1;
    leds_max        = entry && entry->max_leds ? entry->max_leds : 150;
    matrix_width    = 0;
    matrix_height   = 0;
    matrix_map.clear();

    if((entry == nullptr) || (entry->layout == SkydimoLayoutKind::None))
    {
        return;
    }

    resizable   = false;
    leds_count  = entry->total;
    leds_min    = entry->total;
    leds_max    = entry->total;

    if(entry->layout == SkydimoLayoutKind::Strip1)
    {
        return;
    }

    const unsigned int z1 = entry->zones[0];
    const unsigned int z2 = entry->zones[1];
    const unsigned int z3 = entry->zones[2];
    const unsigned int z4 = entry->zones[3];

    if(entry->layout == SkydimoLayoutKind::Perimeter4)
    {
        matrix_height = std::max(z1, z3) + 2;
        matrix_width  = std::max(z2, z4) + 2;
    }
    else if(entry->layout == SkydimoLayoutKind::Perimeter3)
    {
        matrix_height = std::max(z1, z3) + 1;
        matrix_width  = z2 + 2;
    }
    else
    {
        matrix_height = std::max(z1, z2) + 2;
        matrix_width  = std::max(3U, static_cast<unsigned int>(((16.0 / 9.0) * matrix_height) + 0.5));
    }

    matrix_map.assign(matrix_width * matrix_height, SKYDIMO_MATRIX_NA);

    unsigned int led_index = 0;
    auto set_cell = [&](unsigned int y, unsigned int x)
    {
        if((y < matrix_height) && (x < matrix_width))
        {
            matrix_map[(y * matrix_width) + x] = led_index++;
        }
    };

    if(entry->layout == SkydimoLayoutKind::Sides2)
    {
        unsigned int placed = 0;
        int y = static_cast<int>(matrix_height) - 2;

        while((placed < z1) && (y >= 1))
        {
            set_cell(static_cast<unsigned int>(y), 0);
            placed++;
            y--;
        }
    }
    else
    {
        unsigned int placed = 0;
        int y = entry->layout == SkydimoLayoutKind::Perimeter3 ? static_cast<int>(matrix_height) - 1
                                                               : static_cast<int>(matrix_height) - 2;

        while((placed < z1) && (y >= 1))
        {
            set_cell(static_cast<unsigned int>(y), matrix_width - 1);
            placed++;
            y--;
        }
    }

    if((entry->layout == SkydimoLayoutKind::Perimeter3) || (entry->layout == SkydimoLayoutKind::Perimeter4))
    {
        unsigned int placed = 0;
        int x = static_cast<int>(matrix_width) - 2;

        while((placed < z2) && (x >= 1))
        {
            set_cell(0, static_cast<unsigned int>(x));
            placed++;
            x--;
        }
    }
    else if(entry->layout == SkydimoLayoutKind::Sides2)
    {
        unsigned int placed = 0;
        unsigned int y = 1;

        while((placed < z2) && (y <= (matrix_height - 2)))
        {
            set_cell(y, matrix_width - 1);
            placed++;
            y++;
        }
    }

    unsigned int end_y = entry->layout == SkydimoLayoutKind::Perimeter3 ? matrix_height - 1 : matrix_height - 2;
    unsigned int placed = 0;
    unsigned int y = 1;

    while((placed < z3) && (y <= end_y))
    {
        set_cell(y, 0);
        placed++;
        y++;
    }

    if(entry->layout == SkydimoLayoutKind::Perimeter4)
    {
        placed = 0;
        unsigned int x = 1;

        while((placed < z4) && (x <= (matrix_width - 2)))
        {
            set_cell(matrix_height - 1, x);
            placed++;
            x++;
        }
    }
}

bool SkydimoSerialController::WriteFrame(const std::vector<RGBColor>& colors, unsigned int count)
{
    if(!port_open)
    {
        return false;
    }

    count = std::min<unsigned int>(count, static_cast<unsigned int>(colors.size()));

    std::vector<unsigned char> packet;
    packet.reserve(6 + (count * 3));

    packet.push_back(0x41);
    packet.push_back(0x64);
    packet.push_back(0x61);
    packet.push_back(0x00);
    packet.push_back((count / 256) & 0xFF);
    packet.push_back(count & 0xFF);

    for(unsigned int led_idx = 0; led_idx < count; led_idx++)
    {
        packet.push_back(RGBGetRValue(colors[led_idx]));
        packet.push_back(RGBGetGValue(colors[led_idx]));
        packet.push_back(RGBGetBValue(colors[led_idx]));
    }

    return serialport.serial_write(reinterpret_cast<char*>(packet.data()), static_cast<int>(packet.size())) == static_cast<int>(packet.size());
}

void SkydimoSerialController::SetLEDs(const std::vector<RGBColor>& colors)
{
    WriteFrame(colors, leds_count);
}

void SkydimoSerialController::SetBlack()
{
    std::vector<RGBColor> black;

    black.resize(leds_count, ToRGBColor(0, 0, 0));
    WriteFrame(black, leds_count);
}

void SkydimoSerialController::SetLEDCount(unsigned int led_count)
{
    if(!resizable)
    {
        return;
    }

    if((led_count < leds_min) || (led_count > leds_max))
    {
        return;
    }

    leds_count = led_count;
}

std::string SkydimoSerialController::GetName() const
{
    return model_name;
}

std::string SkydimoSerialController::GetSerial() const
{
    return serial_id;
}

std::string SkydimoSerialController::GetLocation() const
{
    return "Serial: " + port_name;
}

bool SkydimoSerialController::IsResizable() const
{
    return resizable;
}

unsigned int SkydimoSerialController::GetLEDCount() const
{
    return leds_count;
}

unsigned int SkydimoSerialController::GetMinLEDCount() const
{
    return leds_min;
}

unsigned int SkydimoSerialController::GetMaxLEDCount() const
{
    return leds_max;
}

unsigned int SkydimoSerialController::GetMatrixWidth() const
{
    return matrix_width;
}

unsigned int SkydimoSerialController::GetMatrixHeight() const
{
    return matrix_height;
}

const std::vector<unsigned int>& SkydimoSerialController::GetMatrixMap() const
{
    return matrix_map;
}
