/*---------------------------------------------------------*\
| RedragonK652Controller.cpp                               |
|                                                          |
|   Experimental controller for the wired Redragon K652    |
|                                                          |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RedragonK652Controller.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <thread>
#include <utility>

namespace
{
    static constexpr std::size_t GREEN_PLANE_OFFSET       = 0x7E;
    static constexpr std::size_t BLUE_PLANE_OFFSET        = 0xFC;
    static constexpr std::size_t DIRECT_SUFFIX_OFFSET     = 0x27C;
    static constexpr std::size_t STATIC_RED_OFFSET        = 0x1D;
    static constexpr std::size_t STATIC_GREEN_OFFSET      = 0x1E;
    static constexpr std::size_t STATIC_BLUE_OFFSET       = 0x1F;

    using RedragonK652Report = std::array<unsigned char, redragon_k652::REPORT_SIZE>;

    static_assert(STATIC_BLUE_OFFSET < redragon_k652::REPORT_SIZE, "K652 static RGB fields must fit in the report");
    static_assert(0x7F + BLUE_PLANE_OFFSET < redragon_k652::REPORT_SIZE, "K652 RGB planes must fit in the report");

    int SendFeatureReport(hid_device* device, const unsigned char* data, std::size_t size)
    {
        return(hid_send_feature_report(device, data, size));
    }

    int GetFeatureReport(hid_device* device, unsigned char* data, std::size_t size)
    {
        return(hid_get_feature_report(device, data, size));
    }

    void CloseDevice(hid_device* device)
    {
        hid_close(device);
    }

    void SleepMilliseconds(unsigned int milliseconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    static const RedragonK652HIDOperations DEFAULT_HID_OPERATIONS =
    {
        SendFeatureReport,
        GetFeatureReport,
        CloseDevice,
        SleepMilliseconds
    };

    /*-----------------------------------------------------------------*\
    | The final 83-key layout is hardware-validated.  Evidence consists |
    | of 44 individually captured indices plus 39 contiguous-derived     |
    | positions between captured row boundaries.                         |
    \*-----------------------------------------------------------------*/
    static constexpr std::array<unsigned char, redragon_k652::LED_COUNT> K652_KEY_INDICES =
    {
        /* Directly captured top row */
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,

        /* Captured: `, 1, Backspace, Home.  2 through = are contiguous-derived. */
        0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,

        /* Captured: Tab, Q, Backslash, End.  W through ] are contiguous-derived. */
        0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,

        /* Captured: CapsLock, A, Quote, Enter, PageUp.  S through ; are contiguous-derived. */
        0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x54, 0x55,

        /* Captured: both Shifts, Z, Slash, Up, PageDown.  X through . are contiguous-derived. */
        0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x68, 0x69, 0x6A,

        /* Directly captured; Fn at the unverified 0x7A position is excluded */
        0x71, 0x72, 0x73, 0x76, 0x79, 0x7B, 0x7D, 0x7E, 0x7F
    };

    /* Reserved/unexposed per-key RGB positions; 0x7A is the Fn candidate. */
    static constexpr std::array<unsigned char, 8> K652_RESERVED_INDICES =
    {
        0x53, 0x67, 0x74, 0x75, 0x77, 0x78, 0x7A, 0x7C
    };

    constexpr bool ValidateKeyIndices()
    {
        for(std::size_t key_idx = 0; key_idx < K652_KEY_INDICES.size(); key_idx++)
        {
            const unsigned char firmware_idx = K652_KEY_INDICES[key_idx];

            if(firmware_idx > 0x7F || firmware_idx + BLUE_PLANE_OFFSET >= redragon_k652::REPORT_SIZE)
            {
                return(false);
            }

            for(unsigned char reserved_idx : K652_RESERVED_INDICES)
            {
                if(firmware_idx == reserved_idx)
                {
                    return(false);
                }
            }

            for(std::size_t prior_idx = 0; prior_idx < key_idx; prior_idx++)
            {
                if(firmware_idx == K652_KEY_INDICES[prior_idx])
                {
                    return(false);
                }
            }
        }

        return(true);
    }

    static_assert(ValidateKeyIndices(), "K652 firmware indices must be unique, non-reserved, and bounds-safe");

    /*-----------------------------------------------------------------*\
    | These reports and sparse masks were extracted from official K652  |
    | software captures.  They intentionally do not reuse the older F11 |
    | templates, which differ at multiple otherwise opaque bytes.       |
    | Opaque fields are preserved; their meanings are not inferred.     |
    \*-----------------------------------------------------------------*/

    static constexpr std::array<unsigned char, 6> CONFIGURATION_QUERY =
    {
        0x05, 0x83, 0xB6, 0x00, 0x00, 0x00
    };

    static constexpr std::array<unsigned char, 128> CAPTURED_DIRECT_SUFFIX =
    {
        0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    static_assert(DIRECT_SUFFIX_OFFSET + CAPTURED_DIRECT_SUFFIX.size() <= redragon_k652::REPORT_SIZE, "K652 direct suffix must fit in the report");

    template<std::size_t N>
    constexpr bool OffsetsFit(const unsigned short (&offsets)[N])
    {
        for(unsigned short offset : offsets)
        {
            if(offset >= redragon_k652::REPORT_SIZE)
            {
                return(false);
            }
        }

        return(true);
    }

    static constexpr unsigned short CAPTURED_STATIC_FF_OFFSETS[] =
    {
        0x0022, 0x0024, 0x0026, 0x0027, 0x0029, 0x002B, 0x002D, 0x002E, 0x002F,
        0x0030, 0x0031, 0x0032, 0x0037, 0x0039, 0x003B, 0x003C, 0x003E,
        0x0040, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x004C, 0x004E,
        0x0050, 0x0051, 0x0053, 0x0055, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C,
        0x0061, 0x0063, 0x0065, 0x0066, 0x0068, 0x006A, 0x006C, 0x006D, 0x006E, 0x006F,
        0x0070, 0x0071, 0x0076, 0x0078, 0x007A, 0x007B, 0x007D, 0x007F,
        0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x008B, 0x008D, 0x008F,
        0x0090, 0x0092, 0x0094, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B,
        0x00A0, 0x00A2, 0x00A4, 0x00A5, 0x00A7, 0x00A9, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
        0x00B0, 0x00B5, 0x00B7, 0x00B9, 0x00BA, 0x00BC, 0x00BE,
        0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00CA, 0x00CC, 0x00CE, 0x00CF,
        0x00D1, 0x00D3, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DF,
        0x00E1, 0x00E3, 0x00E4, 0x00E6, 0x00E8, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
        0x00F4, 0x00F6, 0x00F8, 0x00F9, 0x00FB, 0x00FD, 0x00FF,
        0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0109, 0x010B, 0x010D, 0x010E,
        0x0110, 0x0112, 0x0114, 0x0115, 0x0116, 0x0117, 0x0118, 0x0119, 0x011E,
        0x0120, 0x0122, 0x0123, 0x0125, 0x0127, 0x0129, 0x012A, 0x012B, 0x012C, 0x012D, 0x012E,
        0x0133, 0x0135, 0x0137, 0x0138, 0x013A, 0x013C, 0x013E, 0x013F,
        0x0140, 0x0141, 0x0142, 0x0143, 0x0148, 0x014A, 0x014C, 0x014D, 0x014F,
        0x0151, 0x0153, 0x0154, 0x0155, 0x0156, 0x0157, 0x0158, 0x015D, 0x015F,
        0x0161, 0x0162, 0x0164, 0x0166, 0x0168, 0x0169, 0x016A, 0x016B, 0x016C, 0x016D,
        0x0172, 0x0174, 0x0176, 0x0177, 0x0179, 0x017B, 0x017D, 0x017E, 0x017F,
        0x0180, 0x0181, 0x0182, 0x0187, 0x0189, 0x018B, 0x018C, 0x018E,
        0x0190, 0x0192, 0x0193, 0x0194, 0x0195, 0x0196, 0x0197, 0x019C, 0x019E,
        0x01A0, 0x01A1, 0x01A3, 0x01A5, 0x01A7, 0x01A8, 0x01A9, 0x01AA, 0x01AB, 0x01AC,
        0x01B1, 0x01B3, 0x01B5, 0x01B6, 0x01B8, 0x01BA, 0x01BC, 0x01BD, 0x01BE, 0x01BF,
        0x01C0, 0x01C1, 0x01C6, 0x01C8, 0x01CA, 0x01CB, 0x01CD, 0x01CF,
        0x01D1, 0x01D2, 0x01D3, 0x01D4, 0x01D5, 0x01D6, 0x01DB, 0x01DD, 0x01DF,
        0x01E0, 0x01E2, 0x01E4, 0x01E6, 0x01E7, 0x01E8, 0x01E9, 0x01EA
    };

    static_assert(OffsetsFit(CAPTURED_STATIC_FF_OFFSETS), "K652 static mask offsets must fit in the report");

    static constexpr unsigned short CAPTURED_C0_FF_OFFSETS[] =
    {
        0x00E, 0x00F, 0x010, 0x011, 0x012, 0x013, 0x014,
        0x023, 0x024, 0x025, 0x026, 0x027,
        0x038, 0x039, 0x03A, 0x03B, 0x03C,
        0x04C, 0x04F, 0x050, 0x061,
        0x188, 0x189, 0x18A, 0x18B, 0x18C, 0x18D, 0x18E,
        0x19D, 0x19E, 0x19F, 0x1A0,
        0x1B2, 0x1B3, 0x1B4, 0x1B5,
        0x318, 0x31A, 0x32C, 0x32D, 0x32E, 0x340, 0x34D, 0x355, 0x357,
        0x361, 0x362, 0x363
    };

    static_assert(OffsetsFit(CAPTURED_C0_FF_OFFSETS), "K652 C0 offsets must fit in the report");

    static constexpr std::array<unsigned char, 144> CAPTURED_DIRECT_B6_PREFIX =
    {
        0x06, 0x03, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5,
        0x03, 0x03, 0x03, 0x00, 0x01, 0x15, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x01, 0x02,
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44,
        0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x00, 0x44, 0x00, 0x44, 0x07, 0x44,
        0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x5A, 0xA5,
        0x00, 0x00, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5, 0x03, 0x03, 0x03, 0x00, 0x00
    };

    static_assert(CAPTURED_DIRECT_B6_PREFIX.size() <= redragon_k652::REPORT_SIZE, "K652 B6 prefix must fit in the report");

    RedragonK652Report MakeStaticReport(RGBColor color)
    {
        RedragonK652Report report{};

        report[0x00] = 0x06;
        report[0x01] = 0x08;
        report[0x02] = 0xB8;
        report[0x04] = 0x40;

        report[STATIC_RED_OFFSET]   = RGBGetRValue(color);
        report[STATIC_GREEN_OFFSET] = RGBGetGValue(color);
        report[STATIC_BLUE_OFFSET]  = RGBGetBValue(color);

        for(unsigned short offset : CAPTURED_STATIC_FF_OFFSETS)
        {
            report[offset] = 0xFF;
        }

        return(report);
    }

    RedragonK652Report MakeDirectReport(const std::vector<RGBColor>& colors)
    {
        RedragonK652Report report{};

        report[0x00] = 0x06;
        report[0x01] = 0x09;
        report[0x02] = 0xBC;
        report[0x04] = 0x40;

        std::copy(CAPTURED_DIRECT_SUFFIX.begin(), CAPTURED_DIRECT_SUFFIX.end(), report.begin() + DIRECT_SUFFIX_OFFSET);

        /* Arbitrary 8-bit channels are captured (for example 128/64/32 and 17/34/51), not inferred from primaries. */
        for(std::size_t led_idx = 0; led_idx < K652_KEY_INDICES.size(); led_idx++)
        {
            const std::size_t key_idx = K652_KEY_INDICES[led_idx];

            /*---------------------------------------------------------*\
            | Arbitrary captured values include Space 128/64/32 and     |
            | Right Arrow 17/34/51 in these exact R/G/B planes.          |
            \*---------------------------------------------------------*/
            report[key_idx]                      = RGBGetRValue(colors[led_idx]);
            report[key_idx + GREEN_PLANE_OFFSET] = RGBGetGValue(colors[led_idx]);
            report[key_idx + BLUE_PLANE_OFFSET]  = RGBGetBValue(colors[led_idx]);
        }

        return(report);
    }

    RedragonK652Report MakeC0Report()
    {
        /* Captured C0 invariant report; its field meanings are intentionally not inferred. */
        RedragonK652Report report{};

        report[0x00] = 0x06;
        report[0x01] = 0x09;
        report[0x02] = 0xC0;
        report[0x04] = 0x40;

        for(unsigned short offset : CAPTURED_C0_FF_OFFSETS)
        {
            report[offset] = 0xFF;
        }

        return(report);
    }

    RedragonK652Report MakeModeReport(unsigned char mode)
    {
        RedragonK652Report report{};
        std::copy(CAPTURED_DIRECT_B6_PREFIX.begin(), CAPTURED_DIRECT_B6_PREFIX.end(), report.begin());

        if(mode == redragon_k652::MODE_STATIC)
        {
            /*-------------------------------------------------------------*\
            | These are the only three bytes that differ between captured  |
            | K652 direct and static B6 reports.                            |
            \*-------------------------------------------------------------*/
            report[0x14] = 0x00;
            report[0x15] = redragon_k652::MODE_STATIC;
            report[0x3C] = 0x07;
        }

        return(report);
    }
}

const std::array<unsigned char, redragon_k652::LED_COUNT> redragon_k652::KEY_INDICES =
    K652_KEY_INDICES;

RedragonK652Controller::RedragonK652Controller(hid_device* dev_cmd_handle, hid_device* dev_data_handle, const char* path, std::string dev_name,
                                               RedragonK652DeviceClaim claim, const RedragonK652HIDOperations* operations_ptr)
    : dev_cmd(dev_cmd_handle), dev_data(dev_data_handle), device_claim(std::move(claim)),
      operations(operations_ptr ? *operations_ptr : DEFAULT_HID_OPERATIONS), user_lighting_control_enabled(false), location(path), name(dev_name)
{
}

RedragonK652Controller::~RedragonK652Controller()
{
    operations.close(dev_cmd);

    if(dev_data != dev_cmd)
    {
        operations.close(dev_data);
    }
}

bool RedragonK652Controller::ValidateConfiguration(hid_device* dev_cmd_handle, hid_device* dev_data_handle,
                                                   const RedragonK652HIDOperations* operations_ptr)
{
    const RedragonK652HIDOperations* hid_operations = operations_ptr ? operations_ptr : &DEFAULT_HID_OPERATIONS;

    if(hid_operations->send_feature_report(dev_cmd_handle, CONFIGURATION_QUERY.data(), CONFIGURATION_QUERY.size()) != static_cast<int>(CONFIGURATION_QUERY.size()))
    {
        return(false);
    }

    /* Captured official-software pacing between query and read. */
    hid_operations->sleep_ms(20);

    RedragonK652Report response{};
    response[0] = 0x06;

    const int result = hid_operations->get_feature_report(dev_data_handle, response.data(), response.size());

    /* Captured official-software pacing after the configuration read. */
    hid_operations->sleep_ms(20);

    return(result >= 0x8E
        && response[0x00] == 0x06
        && response[0x01] == 0x83
        && response[0x02] == 0xB6
        && response[0x0E] == 0x5A
        && response[0x0F] == 0xA5
        && response[0x10] == 0x03
        && response[0x11] == 0x03
        && response[0x12] == 0x03
        && response[0x1C] == 0x55
        && response[0x1D] == 0x55
        && response[0x89] == 0x5A
        && response[0x8A] == 0xA5
        && response[0x8B] == 0x03
        && response[0x8C] == 0x03
        && response[0x8D] == 0x03);
}

std::string RedragonK652Controller::GetLocation()
{
    return("HID: " + location);
}

std::string RedragonK652Controller::GetName()
{
    return(name);
}

unsigned int RedragonK652Controller::GetLEDCount()
{
    return(redragon_k652::LED_COUNT);
}

bool RedragonK652Controller::QueryConfiguration()
{
    return(ValidateConfiguration(dev_cmd, dev_data, &operations));
}

bool RedragonK652Controller::SendReport(const RedragonK652Report& report)
{
    if(operations.send_feature_report(dev_data, report.data(), report.size()) != static_cast<int>(report.size()))
    {
        return(false);
    }

    /*-----------------------------------------------------------------*\
    | Official software waits about 50 ms after each completed transfer,|
    | producing the observed roughly 59 ms setup-to-setup interval.      |
    \*-----------------------------------------------------------------*/
    operations.sleep_ms(50);
    return(true);
}

void RedragonK652Controller::EnableUserLightingControl()
{
    user_lighting_control_enabled.store(true);
}

bool RedragonK652Controller::ApplyStatic(RGBColor color)
{
    if(!user_lighting_control_enabled.load())
    {
        return(false);
    }

    const std::lock_guard<std::mutex> lock(access_mutex);

    if(!QueryConfiguration())
    {
        return(false);
    }

    if(!SendReport(MakeStaticReport(color))
    || !SendReport(MakeModeReport(redragon_k652::MODE_STATIC)))
    {
        return(false);
    }

    /* Captured minimum from final B6 completion to the next query is 70 ms. */
    operations.sleep_ms(20);
    return(true);
}

bool RedragonK652Controller::ApplyDirect(const std::vector<RGBColor>& colors)
{
    if(!user_lighting_control_enabled.load())
    {
        return(false);
    }

    const std::lock_guard<std::mutex> lock(access_mutex);

    if(colors.size() != redragon_k652::LED_COUNT || !QueryConfiguration())
    {
        return(false);
    }

    /*-----------------------------------------------------------------*\
    | All observed per-key applies sent BC, C0, then B6 in this order.   |
    | D4 appeared in only a subset of captures and is intentionally not   |
    | sent.                                                               |
    \*-----------------------------------------------------------------*/
    if(!SendReport(MakeDirectReport(colors))
    || !SendReport(MakeC0Report())
    || !SendReport(MakeModeReport(redragon_k652::MODE_DIRECT)))
    {
        return(false);
    }

    /* Captured minimum from final B6 completion to the next query is 70 ms. */
    operations.sleep_ms(20);
    return(true);
}
