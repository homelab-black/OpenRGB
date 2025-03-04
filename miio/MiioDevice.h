#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "Device.h"

#include <string>

class MiioDevice {
public:
    MiioDevice(std::string ip, std::string token);

    json sendCommand(json &command);
private:
    Device device;
};
