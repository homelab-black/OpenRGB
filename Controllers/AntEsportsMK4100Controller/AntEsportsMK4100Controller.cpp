/*---------------------------------------------------------*\
| AntEsportsMK4100Controller.cpp                            |
|                                                           |
| Native HID protocol adapter for Ant Esports MK4100        |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include <algorithm>
#include <atomic>
#include <cstring>
#include <cstdio>
#include <limits>
#include <mutex>
#include "AntEsportsMK4100Controller.h"
#include "StringUtils.h"
#include "LogManager.h"

AntEsportsMK4100Controller::AntEsportsMK4100Controller(hid_device* dev_handle,
                                                       const std::string& path,
                                                       const std::string& dev_name)
    : dev(dev_handle), location(path), name(dev_name), initialized(false)
{
    std::memset(&protocol, 0, sizeof(protocol));

    mk4100_transport transport;
    transport.context = this;
    transport.write_report = &AntEsportsMK4100Controller::WriteReport;
    transport.read_report = &AntEsportsMK4100Controller::ReadReport;

    if(!dev)
    {
        error = "HID handle is null";
        return;
    }

    if(mk4100_init(&protocol, &transport, 500) != 0)
    {
        error = mk4100_last_error(&protocol);
        return;
    }

    initialized = true;
}

AntEsportsMK4100Controller::~AntEsportsMK4100Controller()
{
    /* OpenRGB normally leaves the last selected lighting state active when it
       exits. Do not restore the probe-time snapshot from a device destructor. */
    if(initialized)
    {
        mk4100_deinit(&protocol);
    }

    if(dev)
    {
        hid_close(dev);
        dev = nullptr;
    }
}

int AntEsportsMK4100Controller::WriteReport(void* context, const uint8_t report[MK4100_REPORT_BYTES])
{
    AntEsportsMK4100Controller* self = static_cast<AntEsportsMK4100Controller*>(context);
    if(!self || !self->dev) return -1;

    /* HIDAPI requires the report ID as byte 0. This interface is unnumbered,
       so use ID 0 followed by the 64-byte vendor payload. */
    unsigned char output[MK4100_REPORT_BYTES + 1] = {0};
    std::memcpy(output + 1, report, MK4100_REPORT_BYTES);
    int written = hid_write(self->dev, output, sizeof(output));
    static std::atomic<unsigned int> diagnostic_writes{0};
    const unsigned int diagnostic_index = diagnostic_writes.fetch_add(1);
    if(diagnostic_index < 80)
    {
        char prefix[64] = {0};
        const int limit = std::min(written > 0 ? written : 0, 12);
        int pos = 0;
        for(int i = 0; i < limit && pos < static_cast<int>(sizeof(prefix)) - 4; ++i)
            pos += std::snprintf(prefix + pos, sizeof(prefix) - static_cast<std::size_t>(pos), "%s%02X", i ? " " : "", output[i]);
        LOG_WARNING("[Ant Esports MK4100 TXDIAG] result=%d requested=%zu prefix=[%s]", written, sizeof(output), prefix);
    }
    return written == static_cast<int>(sizeof(output)) ? 0 : -1;
}

int AntEsportsMK4100Controller::ReadReport(void* context,
                                           uint8_t report[MK4100_REPORT_BYTES],
                                           unsigned timeout_ms)
{
    AntEsportsMK4100Controller* self = static_cast<AntEsportsMK4100Controller*>(context);
    if(!self || !self->dev) return -1;

    unsigned char input[MK4100_REPORT_BYTES + 1] = {0};
    const unsigned bounded_timeout = std::min(timeout_ms, static_cast<unsigned>(std::numeric_limits<int>::max()));
    int received = hid_read_timeout(self->dev, input, sizeof(input), static_cast<int>(bounded_timeout));
    static std::atomic<unsigned int> diagnostic_reports{0};
    const unsigned int diagnostic_index = diagnostic_reports.fetch_add(1);
    if(received < 0)
    {
        if(diagnostic_index < 80)
            LOG_WARNING("[Ant Esports MK4100 RXDIAG] hid_read_timeout returned error %d", received);
        return -1;
    }
    if(received == 0)
    {
        if(diagnostic_index < 80 && bounded_timeout >= 400)
            LOG_WARNING("[Ant Esports MK4100 RXDIAG] no report before timeout (wait=%u ms)", bounded_timeout);
        return 0;
    }

    if(diagnostic_index < 80)
    {
        char prefix[64] = {0};
        const int limit = std::min(received, 12);
        int pos = 0;
        for(int i = 0; i < limit && pos < static_cast<int>(sizeof(prefix)) - 4; ++i)
            pos += std::snprintf(prefix + pos, sizeof(prefix) - static_cast<std::size_t>(pos), "%s%02X", i ? " " : "", input[i]);
        LOG_WARNING("[Ant Esports MK4100 RXDIAG] len=%d prefix=[%s] wait=%u ms", received, prefix, bounded_timeout);
    }

    /* HIDAPI reports may include a leading Report ID, depending on backend
       and descriptor. Log each received shape to determine why a response
       is failing the existing parser; preserve current handling for diagnosis. */
    if(received == static_cast<int>(MK4100_REPORT_BYTES + 1))
    {
        if(input[0] == 0)
        {
            std::memcpy(report, input + 1, MK4100_REPORT_BYTES);
            return 1;
        }
        if(diagnostic_index < 80)
            LOG_WARNING("[Ant Esports MK4100 RXDIAG] dropping 65-byte report with unexpected first byte 0x%02X", input[0]);
        return 0;
    }
    if(received == static_cast<int>(MK4100_REPORT_BYTES))
    {
        std::memcpy(report, input, MK4100_REPORT_BYTES);
        return 1;
    }

    if(diagnostic_index < 80)
        LOG_WARNING("[Ant Esports MK4100 RXDIAG] ignoring unexpected report length %d (expected %u or %u)",
                    received, MK4100_REPORT_BYTES, MK4100_REPORT_BYTES + 1);
    return 0;
}

bool AntEsportsMK4100Controller::IsInitialized() const
{
    return initialized;
}

std::string AntEsportsMK4100Controller::GetErrorString() const
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    return error;
}

std::string AntEsportsMK4100Controller::GetDeviceLocation() const
{
    return "HID: " + location;
}

std::string AntEsportsMK4100Controller::GetNameString() const
{
    return name.empty() ? "Ant Esports MK4100" : name;
}

std::string AntEsportsMK4100Controller::GetSerialString() const
{
    if(!dev) return "";
    wchar_t serial_string[128] = {0};
    if(hid_get_serial_number_string(dev, serial_string, 128) != 0) return "";
    return StringUtils::wstring_to_string(serial_string);
}

unsigned int AntEsportsMK4100Controller::GetActiveHardwareMode() const
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    return initialized ? protocol.config[2] : MK4100_CUSTOM_MODE;
}

void AntEsportsMK4100Controller::CopyCurrentRGB(uint8_t rgb[MK4100_RGB_BYTES]) const
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!rgb) return;
    if(initialized) std::memcpy(rgb, protocol.rgb, MK4100_RGB_BYTES);
    else std::memset(rgb, 0, MK4100_RGB_BYTES);
}

void AntEsportsMK4100Controller::CopyCurrentConfig(uint8_t config[MK4100_CONFIG_BYTES]) const
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!config) return;
    if(initialized) std::memcpy(config, protocol.config, MK4100_CONFIG_BYTES);
    else std::memset(config, 0, MK4100_CONFIG_BYTES);
}

bool AntEsportsMK4100Controller::ReadConfig(uint8_t config[MK4100_CONFIG_BYTES])
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized || !config) return false;
    if(mk4100_read_config(&protocol, config) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}

bool AntEsportsMK4100Controller::ReadEffectConfigForMode(unsigned int mode, uint8_t config[MK4100_CONFIG_BYTES])
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized || !config) return false;
    if(mk4100_read_effect_config_for_mode(&protocol, mode, config) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}

bool AntEsportsMK4100Controller::WriteConfig(const uint8_t config[MK4100_CONFIG_BYTES])
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized || !config) return false;
    if(mk4100_write_config(&protocol, config) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}

bool AntEsportsMK4100Controller::SelectCustomMode()
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized) return false;
    if(mk4100_select_custom_mode(&protocol) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}

bool AntEsportsMK4100Controller::SetSingleKeyRGB(unsigned int slot, uint8_t red, uint8_t green, uint8_t blue)
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized) return false;
    if(mk4100_set_key_rgb(&protocol, slot, red, green, blue) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}

bool AntEsportsMK4100Controller::UpdateRGBFrame(const uint8_t rgb[MK4100_RGB_BYTES], bool verify_after_write)
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized || !rgb) return false;
    mk4100_stats stats;
    if(mk4100_update_rgb_frame(&protocol, rgb, verify_after_write ? 1 : 0, &stats) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}


bool AntEsportsMK4100Controller::ApplyDirectFrame(const uint8_t rgb[MK4100_RGB_BYTES], bool verify_after_write)
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized || !rgb) return false;
    if(mk4100_select_custom_mode(&protocol) != 0 ||
       mk4100_update_rgb_frame(&protocol, rgb, verify_after_write ? 1 : 0, nullptr) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}

bool AntEsportsMK4100Controller::RestoreStartupState()
{
    std::lock_guard<std::mutex> lock(protocol_mutex);
    if(!initialized) return false;
    if(mk4100_restore(&protocol) != 0)
    {
        error = mk4100_last_error(&protocol);
        return false;
    }
    error.clear();
    return true;
}
