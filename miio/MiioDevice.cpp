#include "MiioDevice.h"

MiioDevice::MiioDevice(std::string ip, std::string token) : device(ip.c_str(), token.c_str())
{
}

json MiioDevice::sendCommand(json &command) {
    return device.send(command);
}
