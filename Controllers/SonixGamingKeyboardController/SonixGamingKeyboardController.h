/*---------------------------------------------------------*\
| SonixGamingKeyboardController.h                           |
|                                                           |
|   Driver for Sonix based keyboards using the "Gaming      |
|   Force Z" configuration software (CZC.Gaming Revenant)   |
|                                                           |
|   Roman Ondráček                              05 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| All communication uses 64 byte feature reports without a  |
| report ID                                                 |
\*---------------------------------------------------------*/
#define SONIX_GK_PACKET_SIZE                64

/*---------------------------------------------------------*\
| Lighting data block layout                                |
|   0x000 - 0x1FF   32 mode records, 16 bytes each         |
|   0x200 - 0x43F   8 x 18 key matrix, 4 bytes per key     |
|                   (0x80, R, G, B)                         |
|   0x440 - 0x44F   Active mode record                      |
\*---------------------------------------------------------*/
#define SONIX_GK_LIGHTING_DATA_SIZE         0x480
#define SONIX_GK_MODE_RECORD_SIZE           16
#define SONIX_GK_MODE_RECORD_COUNT          32
#define SONIX_GK_KEY_DATA_OFFSET            0x200
#define SONIX_GK_ACTIVE_RECORD_OFFSET       0x440
#define SONIX_GK_MATRIX_ROWS                8
#define SONIX_GK_MATRIX_COLUMNS             18

enum
{
    SONIX_GK_COMMAND_GET_CHECKSUM           = 0x02,     /* Read checksum of last block      */
    SONIX_GK_COMMAND_WRITE_KEYMAP           = 0x11,     /* Key assignment block             */
    SONIX_GK_COMMAND_WRITE_LIGHTING         = 0x13,     /* Lighting block                   */
    SONIX_GK_COMMAND_WRITE_MACROS           = 0x15,     /* Macro block                      */
};

enum
{
    SONIX_GK_MODE_STATIC                    = 0x01,
    SONIX_GK_MODE_SINGLE_ON                 = 0x02,
    SONIX_GK_MODE_SINGLE_OFF                = 0x03,
    SONIX_GK_MODE_GLITTERING                = 0x04,
    SONIX_GK_MODE_FALLING                   = 0x05,
    SONIX_GK_MODE_COLOURFUL                 = 0x06,
    SONIX_GK_MODE_BREATH                    = 0x07,
    SONIX_GK_MODE_SPECTRUM                  = 0x08,
    SONIX_GK_MODE_OUTWARD                   = 0x09,
    SONIX_GK_MODE_SCROLLING                 = 0x0A,
    SONIX_GK_MODE_ROLLING                   = 0x0B,
    SONIX_GK_MODE_ROTATING                  = 0x0C,
    SONIX_GK_MODE_EXPLODE                   = 0x0D,
    SONIX_GK_MODE_LAUNCH                    = 0x0E,
    SONIX_GK_MODE_RIPPLES                   = 0x0F,
    SONIX_GK_MODE_FLOWING                   = 0x10,
    SONIX_GK_MODE_PULSATING                 = 0x11,
    SONIX_GK_MODE_TILT                      = 0x12,
    SONIX_GK_MODE_SHUTTLE                   = 0x13,
    SONIX_GK_MODE_CUSTOM                    = 0x80,
};

#define SONIX_GK_BUILTIN_MODE_COUNT         19
#define SONIX_GK_CUSTOM_RECORD_INDEX        19

#define SONIX_GK_BRIGHTNESS_MIN             1
#define SONIX_GK_BRIGHTNESS_MAX             16
#define SONIX_GK_SPEED_MIN                  1
#define SONIX_GK_SPEED_MAX                  16
#define SONIX_GK_SPEED_DEFAULT              12

struct sonix_gk_mode_record
{
    unsigned char   mode;
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;
    bool            random;
    unsigned char   brightness;
    unsigned char   speed;
    unsigned char   direction;
};

class SonixGamingKeyboardController
{
public:
    SonixGamingKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~SonixGamingKeyboardController();

    std::string GetDeviceLocation();
    std::string GetNameString();
    std::string GetSerialString();

    /*-----------------------------------------------------*\
    | records:     one record per lighting data slot        |
    | active:      index of the active record               |
    | key_colors:  SONIX_GK_MATRIX_ROWS *                   |
    |              SONIX_GK_MATRIX_COLUMNS colors           |
    \*-----------------------------------------------------*/
    bool        SetLighting
                    (
                    const sonix_gk_mode_record* records,
                    unsigned int                record_count,
                    unsigned int                active,
                    const RGBColor*             key_colors
                    );

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;

    void            BuildModeRecord(unsigned char* buf, const sonix_gk_mode_record& record);

    bool            SendPacket(const unsigned char* data);
    bool            ReadPacket(unsigned char* data);
    bool            SendCommand(unsigned char command);
    bool            WaitForAck(unsigned char* response);

    bool            WriteBlock(unsigned char command, const unsigned char* data, unsigned int size);
};
