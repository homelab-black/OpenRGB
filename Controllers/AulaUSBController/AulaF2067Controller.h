#pragma once

#include <hidapi.h>
#include <string>

#define AULA_F2067_BRIGHTNESS_MIN   0
#define AULA_F2067_BRIGHTNESS_MAX   5
#define AULA_F2067_BRIGHTNESS       3

#define AULA_F2067_SPEED_MIN        0
#define AULA_F2067_SPEED_MAX        2
#define AULA_F2067_SPEED            1

class AulaF2067Controller
{
public:
    AulaF2067Controller(hid_device* dev_handle, const char* dt_path);
    ~AulaF2067Controller();

    std::string GetDeviceLocation();

    void SendPacket();

    void setBrightness(unsigned char brightness);
    void setSpeed(unsigned char speed);
    void setAnimationEffect(unsigned char animation_effect);

private:
    hid_device* dev;
    std::string location;

    unsigned char brightness = 0x04;
    unsigned char speed = 0x01;
    unsigned char animation_effect = 0x00;
};