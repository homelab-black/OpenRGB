/*---------------------------------------------------------*\
| AntEsportsMK4100Controller.h                              |
|                                                           |
| Controller for Ant Esports MK4100 keyboard                |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <hidapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MK4100_VID               0x36AEu
#define MK4100_PID               0xFE9Cu
#define MK4100_REPORT_BYTES      64u
#define MK4100_CONFIG_BYTES      11u
#define MK4100_RGB_SLOTS         126u
#define MK4100_RGB_BYTES         (MK4100_RGB_SLOTS * 3u)
#define MK4100_PHYSICAL_KEYS     104u
#define MK4100_RGB_CHUNK_BYTES   56u
#define MK4100_CUSTOM_MODE       16u

/*
 * Transport contract:
 * - write_report receives the 64-byte vendor payload, without a report-ID byte.
 *   A HIDAPI adapter must prepend report ID 0 when calling hid_write().
 * - read_report returns 1 for one complete 64-byte input report, 0 for timeout
 *   or a discarded/short report, and -1 for a transport error.
 * - All transport calls are synchronous. Each request waits for its matching
 *   keyboard-protocol response before returning.
 */
typedef int (*mk4100_write_report_fn)(void *context,
                                      const uint8_t report[MK4100_REPORT_BYTES]);
typedef int (*mk4100_read_report_fn)(void *context,
                                     uint8_t report[MK4100_REPORT_BYTES],
                                     unsigned timeout_ms);

typedef struct mk4100_transport {
    void *context;
    mk4100_write_report_fn write_report;
    mk4100_read_report_fn read_report;
} mk4100_transport;

typedef struct mk4100_stats {
    unsigned chunks_sent;
    unsigned bytes_sent;
    double elapsed_ms;
} mk4100_stats;

typedef struct mk4100_device {
    mk4100_transport transport;
    unsigned timeout_ms;
    uint8_t config[MK4100_CONFIG_BYTES];
    uint8_t original_config[MK4100_CONFIG_BYTES];
    uint8_t rgb[MK4100_RGB_BYTES];
    uint8_t original_rgb[MK4100_RGB_BYTES];
    int config_trusted;
    int rgb_trusted;
    int initialized;
    char error[256];
} mk4100_device;

/* Read configuration and two stable RGB snapshots; preserves startup state. */
int mk4100_init(mk4100_device *device, const mk4100_transport *transport,
                unsigned timeout_ms);
void mk4100_deinit(mk4100_device *device);
const char *mk4100_last_error(const mk4100_device *device);

int mk4100_read_config(mk4100_device *device,
                       uint8_t config_out[MK4100_CONFIG_BYTES]);
/* Query the vendor's mode-specific configuration template (command 0x16).
   This does not itself change the active lighting mode. */
int mk4100_read_effect_config_for_mode(mk4100_device *device, unsigned mode,
                                       uint8_t config_out[MK4100_CONFIG_BYTES]);
int mk4100_write_config(mk4100_device *device,
                        const uint8_t config[MK4100_CONFIG_BYTES]);
int mk4100_select_custom_mode(mk4100_device *device);
int mk4100_set_brightness(mk4100_device *device, unsigned brightness); /* 0..4 */

int mk4100_read_rgb(mk4100_device *device, uint8_t rgb_out[MK4100_RGB_BYTES]);
int mk4100_refresh_rgb(mk4100_device *device, int require_stable_double_read);
int mk4100_set_key_rgb(mk4100_device *device, unsigned slot,
                        uint8_t red, uint8_t green, uint8_t blue);

/*
 * Serial, acknowledged live RGB update using 54-byte RGB-aligned chunks.
 * Requires custom/UserLight mode to be active; never changes lighting config.
 * Chunks are sent descending with a two-byte overlap for firmware that may
 * copy the full 56-byte data area. Known fragile boundary keys are repaired
 * whenever a sent chunk touches their RGB triplet, even if their color was not
 * part of the requested change.
 * The 56-byte helper remains for reads and restore/static operations.
 * verify_after_write enables a full RGB readback (normally disabled in OpenRGB).
 */
int mk4100_update_rgb_frame(mk4100_device *device,
                            const uint8_t rgb[MK4100_RGB_BYTES],
                            int verify_after_write,
                            mk4100_stats *stats);

/* Safely replace frame: hide LEDs, write serially, verify, then enable custom. */
int mk4100_set_static_rgb_frame(mk4100_device *device,
                                const uint8_t rgb[MK4100_RGB_BYTES],
                                mk4100_stats *stats);
/* Restore the original startup buffer/config safely; on failures LEDs may stay off. */
int mk4100_restore(mk4100_device *device);

#ifdef __cplusplus
}
#endif

class AntEsportsMK4100Controller
{
public:
    AntEsportsMK4100Controller(hid_device* dev_handle, const std::string& path, const std::string& name);
    ~AntEsportsMK4100Controller();

    bool IsInitialized() const;
    std::string GetErrorString() const;
    std::string GetDeviceLocation() const;
    std::string GetNameString() const;
    std::string GetSerialString() const;

    unsigned int GetActiveHardwareMode() const;
    void CopyCurrentRGB(uint8_t rgb[MK4100_RGB_BYTES]) const;
    void CopyCurrentConfig(uint8_t config[MK4100_CONFIG_BYTES]) const;

    bool ReadConfig(uint8_t config[MK4100_CONFIG_BYTES]);
    bool ReadEffectConfigForMode(unsigned int mode, uint8_t config[MK4100_CONFIG_BYTES]);
    bool WriteConfig(const uint8_t config[MK4100_CONFIG_BYTES]);
    bool SelectCustomMode();
    bool SetSingleKeyRGB(unsigned int slot, uint8_t red, uint8_t green, uint8_t blue);
    bool UpdateRGBFrame(const uint8_t rgb[MK4100_RGB_BYTES], bool verify_after_write = false);
    bool ApplyDirectFrame(const uint8_t rgb[MK4100_RGB_BYTES], bool verify_after_write = false);
    bool RestoreStartupState();

private:
    static int WriteReport(void* context, const uint8_t report[MK4100_REPORT_BYTES]);
    static int ReadReport(void* context, uint8_t report[MK4100_REPORT_BYTES], unsigned timeout_ms);

    hid_device* dev;
    std::string location;
    std::string name;
    std::string error;
    mk4100_device protocol;
    bool initialized;
};
