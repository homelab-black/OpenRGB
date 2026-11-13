/*---------------------------------------------------------*\
| CosmicByteFireflyKeyboardController.cpp                   |
|                                                           |
|   Driver for Cosmic Byte Firefly TKL Keyboard             |
|   (Configured for Animations, Static RGB & Per-Key RGB)   |
|                                                           |
|   Akankshya Das                               25 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "CosmicByteFireflyKeyboardController.h"
#include "LogManager.h"
#include <cstring>

#define HID_PACKET_SIZE 64

CosmicByteFireflyKeyboardController::CosmicByteFireflyKeyboardController(hid_device* dev_handle, const char* path, unsigned short pid, std::string name)
{
    dev         = dev_handle;
    location    = path ? path : "";
    product_id  = pid;
    device_name = name;
}

CosmicByteFireflyKeyboardController::~CosmicByteFireflyKeyboardController()
{
    if(dev)
    {
        hid_close(dev);
    }
}

std::string CosmicByteFireflyKeyboardController::GetLocation()
{
    return location;
}

std::string CosmicByteFireflyKeyboardController::GetName()
{
    return device_name;
}

void CosmicByteFireflyKeyboardController::SendInitialize()
{
    if(!dev) return;

    unsigned char usb_buf[9] = { 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA, 0x00 };

    int res = hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
    if(res < 0)
    {
        LOG_ERROR("[FireflyController] SendInitialize failed!");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}
void CosmicByteFireflyKeyboardController::SendControlInstruction(unsigned char brightness, unsigned char speed, unsigned char direction, unsigned char animation_type, unsigned char slider_no)
{
    if(!dev) return;

    unsigned char usb_buf[9] = { 0x00 };
    usb_buf[0] = 0x00;           
    usb_buf[1] = 0x08;          
    usb_buf[2] = animation_type; 
    usb_buf[3] = brightness;     
    usb_buf[4] = speed;          
    usb_buf[5] = direction;      
    usb_buf[6] = slider_no;      
    usb_buf[7] = 0xC4;
    usb_buf[8] = 0x36;

    int res = hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
    if(res < 0)
    {
        LOG_ERROR("[FireflyController] SendControlInstruction failed!");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

void CosmicByteFireflyKeyboardController::SendStaticColorPayload(unsigned char red, unsigned char green, unsigned char blue)
{
    if(!dev) return;

    unsigned char payload[65];
    memset(payload, 0x00, sizeof(payload));

    payload[0] = 0x00; // Report ID Prefix for hid_write

    unsigned char scaled_r = red / 4;
    unsigned char scaled_g = green / 4;
    unsigned char scaled_b = blue / 4;

    for(int slider = 0; slider < 7; slider++)
    {
        int offset = 1 + (slider * 3);
        payload[offset]     = scaled_r;
        payload[offset + 1] = scaled_g;
        payload[offset + 2] = scaled_b;
    }

    int res = hid_write(dev, payload, sizeof(payload));
    if(res < 0)
    {
        LOG_ERROR("[FireflyController] SendStaticColorPayload hid_write failed!");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
void CosmicByteFireflyKeyboardController::SendStaticColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char brightness)
{
    if(!dev) return;

    SendInitialize();
    SendStaticColorPayload(red, green, blue);
    SendControlInstruction(brightness, 0x01, 0x00, 0x00, 0x00);
    SendTerminate();
}
void CosmicByteFireflyKeyboardController::SendTerminate()
{
    if(!dev) return;

    unsigned char term1[9] = { 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA, 0x00 };
    hid_send_feature_report(dev, term1, sizeof(term1));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    unsigned char term2[9] = { 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA, 0x00 };
    hid_send_feature_report(dev, term2, sizeof(term2));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}
void CosmicByteFireflyKeyboardController::SendChannelSetup(unsigned char channel_id)
{
    if(!dev) return;

    LOG_DEBUG("[FireflyController] Sending Channel Setup for Channel: 0x%02X", channel_id);

    unsigned char usb_buf[9] = { 0x00, 0x12, 0x00, 0x01, channel_id, 0x00, 0x55, 0xAA, 0x00 };
    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}
void CosmicByteFireflyKeyboardController::SendDataPayload(const unsigned char* channel_bytes, size_t length)
{
    if(!dev || !channel_bytes) return;

    unsigned char data_pkt[65];
    memset(data_pkt, 0x00, sizeof(data_pkt));

    data_pkt[0] = 0x00; 
    int payload_idx = 1;
    int chunk_count = 0;

    for(size_t i = 0; i < length; i++)
    {
        data_pkt[payload_idx++] = channel_bytes[i];

        if(payload_idx > 64)
        {
            int res = hid_write(dev, data_pkt, sizeof(data_pkt));
            chunk_count++;
            LOG_DEBUG("[FireflyController] Sent Payload Chunk %d (64 bytes), res = %d", chunk_count, res);

            memset(data_pkt, 0x00, sizeof(data_pkt));
            data_pkt[0] = 0x00;
            payload_idx = 1;
        }
    }

    if(payload_idx > 1)
    {
        int res = hid_write(dev, data_pkt, sizeof(data_pkt));
        chunk_count++;
        LOG_DEBUG("[FireflyController] Sent Payload Chunk %d (%d trailing bytes), res = %d", chunk_count, (payload_idx - 1), res);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
}
void CosmicByteFireflyKeyboardController::SendKeyboardColors(const std::vector<RGBColor>& colors)
{
    if(!dev || colors.empty()) return;

    // Step 1: Initial Handshake
    SendInitialize();

    // Prepare 128-byte frame buffers
    unsigned char key_enable_plane[128];
    unsigned char red_plane[128];
    unsigned char green_plane[128];
    unsigned char blue_plane[128];

    memset(key_enable_plane, 0x00, sizeof(key_enable_plane));
    memset(red_plane, 0x00, sizeof(red_plane));
    memset(green_plane, 0x00, sizeof(green_plane));
    memset(blue_plane, 0x00, sizeof(blue_plane));

    // Hardware Offset Map for all 87 OpenRGB matrix keys
    // Structured as a C array to meet fixed list guidelines
   // Exact Hardware Byte Offset Mapping (87 Keys)
    static const int hw_map[87][4] = {
        // --- Row 1 (0 to 15) ---
        {  0, -1, -1, -1 }, // 0:  Esc
        {  1, -1, -1, -1 }, // 1:  F1
        {  2, -1, -1, -1 }, // 2:  F2
        {  3, -1, -1, -1 }, // 3:  F3
        {  4, -1, -1, -1 }, // 4:  F4
        {  5, -1, -1, -1 }, // 5:  F5
        {  6, -1, -1, -1 }, // 6:  F6
        {  7, -1, -1, -1 }, // 7:  F7
        {  8, -1, -1, -1 }, // 8:  F8
        {  9, -1, -1, -1 }, // 9:  F9
        { 10, -1, -1, -1 }, // 10: F10
        { 11, -1, -1, -1 }, // 11: F11
        { 12, -1, -1, -1 }, // 12: F12
        { 13, -1, -1, -1 }, // 13: Prt
        { 14, -1, -1, -1 }, // 14: Scr
        { 15, -1, -1, -1 }, // 15: Pau

        // --- Row 2 (16 to 32) ---
        { 16, -1, -1, -1 }, // 16: ~
        { 17, -1, -1, -1 }, // 17: 1
        { 18, -1, -1, -1 }, // 18: 2
        { 19, -1, -1, -1 }, // 19: 3
        { 20, -1, -1, -1 }, // 20: 4
        { 21, -1, -1, -1 }, // 21: 5
        { 22, -1, -1, -1 }, // 22: 6
        { 23, -1, -1, -1 }, // 23: 7
        { 24, -1, -1, -1 }, // 24: 8
        { 25, -1, -1, -1 }, // 25: 9
        { 26, -1, -1, -1 }, // 26: 0
        { 27, -1, -1, -1 }, // 27: -
        { 28, -1, -1, -1 }, // 28: =
        { 29, -1, -1, -1 }, // 29: Backspace
        { 30, -1, -1, -1 }, // 30: Ins
        { 31, -1, -1, -1 }, // 31: Home
        { 62, -1, -1, -1 }, // 32: Page Up (Hardware Offset 62)

        // --- Row 3 (33 to 49) ---
        { 32, -1, -1, -1 }, // 33: Tab
        { 33, -1, -1, -1 }, // 34: Q
        { 34, -1, -1, -1 }, // 35: W
        { 35, -1, -1, -1 }, // 36: E
        { 36, -1, -1, -1 }, // 37: R
        { 37, -1, -1, -1 }, // 38: T
        { 38, -1, -1, -1 }, // 39: Y
        { 39, -1, -1, -1 }, // 40: U
        { 40, -1, -1, -1 }, // 41: I
        { 41, -1, -1, -1 }, // 42: O
        { 42, -1, -1, -1 }, // 43: P
        { 43, -1, -1, -1 }, // 44: [
        { 44, -1, -1, -1 }, // 45: ]
        { 45, -1, -1, -1 }, // 46: Backslash (\)
        { 46, -1, -1, -1 }, // 47: Del
        { 47, -1, -1, -1 }, // 48: End
        { 63, -1, -1, -1 }, // 49: Page Down (Hardware Offset 63)

        // --- Row 4 (50 to 62) ---
        { 48, -1, -1, -1 }, // 50: Caps Lock
        { 49, -1, -1, -1 }, // 51: A
        { 50, -1, -1, -1 }, // 52: S
        { 51, -1, -1, -1 }, // 53: D
        { 52, -1, -1, -1 }, // 54: F
        { 53, -1, -1, -1 }, // 55: G
        { 54, -1, -1, -1 }, // 56: H
        { 55, -1, -1, -1 }, // 57: J
        { 56, -1, -1, -1 }, // 58: K (Hardware Offset 56)
        { 57, -1, -1, -1 }, // 59: L
        { 58, -1, -1, -1 }, // 60: ;
        { 59, -1, -1, -1 }, // 61: '
        { 60, 61, -1, -1 }, // 62: Enter (2 bytes)

        // --- Row 5 (63 to 75) [Packet 2 starts at 64] ---
        { 64, -1, -1, -1 }, // 63: LShift
        { 65, -1, -1, -1 }, // 64: Z
        { 66, -1, -1, -1 }, // 65: X
        { 67, -1, -1, -1 }, // 66: C
        { 68, -1, -1, -1 }, // 67: V
        { 69, -1, -1, -1 }, // 68: B
        { 70, -1, -1, -1 }, // 69: N
        { 71, -1, -1, -1 }, // 70: M
        { 72, -1, -1, -1 }, // 71: ,
        { 73, -1, -1, -1 }, // 72: .
        { 74, -1, -1, -1 }, // 73: /
        { 75, 76, -1, -1 }, // 74: RShift (2 bytes)
        { 78, -1, -1, -1 }, // 75: Up Arrow

        // --- Row 6 (76 to 86) ---
        { 80, -1, -1, -1 }, // 76: LCtrl
        { 81, -1, -1, -1 }, // 77: Win
        { 82, -1, -1, -1 }, // 78: Alt
        { 85, -1, -1, -1 }, // 79: Space
        { 88, -1, -1, -1 }, // 80: RAlt
        { 89, -1, -1, -1 }, // 81: FN
        { 90, -1, -1, -1 }, // 82: App
        { 91, -1, -1, -1 }, // 83: RCtrl
        { 94, -1, -1, -1 }, // 84: Left Arrow
        { 95, -1, -1, -1 }, // 85: Down Arrow
        { 79, -1, -1, -1 }  // 86: Right Arrow (Hardware Offset 79)
    };
    
    for(size_t i = 0; i < colors.size() && i < 87; i++)
    {
        unsigned char r = RGBGetRValue(colors[i]) / 4;
        unsigned char g = RGBGetGValue(colors[i]) / 4;
        unsigned char b = RGBGetBValue(colors[i]) / 4;

        for(int j = 0; j < 4; j++)
        {
            int offset = hw_map[i][j];
            
            if(offset >= 0 && offset < 128)
            {
                if(r > 0 || g > 0 || b > 0)
                {
                    key_enable_plane[offset] = 0x10;
                    red_plane[offset]        = r;
                    green_plane[offset]      = g;
                    blue_plane[offset]       = b;
                }
            }
        }
    }
    SendChannelSetup(0x00);
    SendDataPayload(key_enable_plane, sizeof(key_enable_plane));

    SendChannelSetup(0x01);
    SendDataPayload(red_plane, sizeof(red_plane));

    SendChannelSetup(0x02);
    SendDataPayload(green_plane, sizeof(green_plane));
    
    SendChannelSetup(0x03);
    SendDataPayload(blue_plane, sizeof(blue_plane));

    SendControlInstruction(0x3F, 0x01, 0x00, 0x34, 0x05);

    SendTerminate();
}