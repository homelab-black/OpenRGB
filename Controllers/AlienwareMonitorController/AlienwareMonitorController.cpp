#include <chrono>
#include <cstring>
#include <thread>
#include "AlienwareMonitorController.h"

AlienwareMonitorController::AlienwareMonitorController(hid_device *dev_handle, const char *path)
{
    dev         = dev_handle;
    location    = path;

    Initialize();
}

AlienwareMonitorController::~AlienwareMonitorController()
{
    hid_close(dev);
}

std::string AlienwareMonitorController::GetLocation()
{
    return("HID: " + location);
}

std::string AlienwareMonitorController::GetSerialString()
{
    return("");
}

void fillInChecksum(unsigned char *packet)
{
    unsigned char checksum = 110;

    for(unsigned int index = 5; index <= 13; index++)
    {
      checksum ^= packet[index];
    }

    packet[14] = checksum;
}

void AlienwareMonitorController::SendColor(unsigned char led_id, unsigned char r, unsigned char g, unsigned char b)
{
    unsigned char packet[65];

    memset(packet, 0xFF, sizeof(packet));

    packet[1] = 0x92;
    packet[2] = 0x37;
    packet[3] = 0x0a;
    packet[4] = 0x00;
    packet[5] = 0x51;
    packet[6] = 0x87;
    packet[7] = 0xd0;
    packet[8] = 0x04;

    packet[9] = led_id;
    packet[10] = r;
    packet[11] = g;
    packet[12] = b;
    packet[13] = 0x64;

    fillInChecksum(packet);

    hid_write(dev, packet, sizeof(packet));

    /*-----------------------------------------------------*\
    | Delay 100 milliseconds                                 |
    \*-----------------------------------------------------*/
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void AlienwareMonitorController::Initialize()
{
    unsigned char packet[10];

    packet[1] = 0x92;
    packet[2] = 0x37;
    packet[3] = 0x05;
    packet[4] = 0x00;
    packet[5] = 0x51;
    packet[6] = 0x82;
    packet[7] = 0xd0;
    packet[8] = 0xf4;
    packet[9] = 0x99;

    hid_write(dev, packet, 65);

    /*-----------------------------------------------------*\
    | Delay 100 milliseconds                                 |
    \*-----------------------------------------------------*/
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
