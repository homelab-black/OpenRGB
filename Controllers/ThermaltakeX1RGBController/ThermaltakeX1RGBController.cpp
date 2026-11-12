#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>
#include "StringUtils.h"
#include "ThermaltakeX1RGBController.h"

/* Protocol LED addresses observed in the public Pulse capture.  Their physical key mapping is
 * intentionally left to the host mapper rather than guessed. */
static const unsigned char x1_led_ids[THERMALTAKE_X1_LED_COUNT] =
{
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x36,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x40,0x42,
    0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,
    0x53,0x54,0x55,0x56,0x57,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x66,
    0x67,0x68,0x69,0x6A,0x6B,0x6D,0x6E,0x70,0x71,0x72,0x74,0x75,0x7D,0x7E
};

ThermaltakeX1RGBController::ThermaltakeX1RGBController(hid_device* dev_handle, const char* path)
{
    dev = dev_handle;
    location = path;
    Initialize();
}

ThermaltakeX1RGBController::~ThermaltakeX1RGBController()
{
    hid_close(dev);
}

std::string ThermaltakeX1RGBController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string ThermaltakeX1RGBController::GetSerialString()
{
    wchar_t serial_string[128];
    return(hid_get_serial_number_string(dev, serial_string, 128) == 0
        ? StringUtils::wstring_to_string(serial_string) : "");
}

bool ThermaltakeX1RGBController::Write(const unsigned char packet[64])
{
    /* The X1 descriptor has no Report ID.  HIDAPI still requires a leading
     * zero report-ID byte before the 64-byte USB report. */
    unsigned char hid_packet[65] = { 0 };
    memcpy(&hid_packet[1], packet, 64);
    return(hid_write(dev, hid_packet, sizeof(hid_packet)) == sizeof(hid_packet));
}

void ThermaltakeX1RGBController::DrainResponses()
{
    unsigned char response[65] = {};
    for(unsigned int i = 0; i < 32; i++)
    {
        int result = hid_read_timeout(dev, response, sizeof(response), 20);
        if(result <= 0) break;
    }
}

void ThermaltakeX1RGBController::Initialize()
{
    /* X1 RGB.exe sends this connection/query sequence before lighting writes.
     * The first state transition can report a HIDAPI timeout even though the
     * keyboard accepts it; continue once, matching the proven host replay. */
    static const unsigned char commands[][5] =
    {
        { 0x41, 0x03, 0x00, 0x00, 0x00 },
        { 0x43, 0x00, 0x00, 0x00, 0x01 },
        { 0x51, 0x00, 0x00, 0x00, 0x00 },
        { 0x50, 0x00, 0x00, 0x00, 0x00 },
        { 0x12, 0x22, 0x00, 0x00, 0x00 },
        { 0x52, 0x92, 0x00, 0x00, 0x00 },
        { 0x52, 0x10, 0x00, 0x00, 0x00 }
    };
    for(unsigned int command_index = 0; command_index < sizeof(commands) / sizeof(commands[0]); command_index++)
    {
        unsigned char packet[64] = {};
        memcpy(packet, commands[command_index], sizeof(commands[command_index]));
        if(!Write(packet))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    DrainResponses();
}

void ThermaltakeX1RGBController::SetStatic(RGBColor color)
{
    unsigned char packet[64] = {};
    packet[0] = 0xC0;
    packet[1] = 0xF0;
    if(!Write(packet)) return;
    memset(packet, 0, sizeof(packet));
    packet[0] = 0xC0;
    /* Capture payload is: C0 00 00 00 R G B ... . */
    packet[4] = RGBGetRValue(color);
    packet[5] = RGBGetGValue(color);
    packet[6] = RGBGetBValue(color);
    if(!Write(packet)) return;
    memset(packet, 0, sizeof(packet));
    packet[0] = 0xC0;
    packet[1] = 0xF0;
    Write(packet);
    DrainResponses();
}

void ThermaltakeX1RGBController::SetLEDs(std::vector<RGBColor> colors)
{
    unsigned char packet[64] = { 0 };
    packet[0] = 0xC0;
    packet[1] = 0xF0;
    if(!Write(packet)) return;

    for(unsigned int first = 0; first < THERMALTAKE_X1_LED_COUNT; first += 15)
    {
        memset(packet, 0, sizeof(packet));
        packet[0] = 0xC0;
        packet[1] = 0x01;
        packet[2] = std::min(15U, THERMALTAKE_X1_LED_COUNT - first);

        for(unsigned int i = 0; i < packet[2]; i++)
        {
            RGBColor color = colors[first + i];
            packet[4 + i * 4] = x1_led_ids[first + i];
            packet[5 + i * 4] = RGBGetRValue(color);
            packet[6 + i * 4] = RGBGetGValue(color);
            packet[7 + i * 4] = RGBGetBValue(color);
        }

        if(!Write(packet)) return;
    }
}
