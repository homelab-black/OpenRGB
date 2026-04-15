/*---------------------------------------------------------*\
| JsonUtils.h                                               |
|                                                           |
|   JSON utility functions                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "nlohmann/json.hpp"

class JsonUtils
{
public:
    static bool JsonParse(std::string json_string, nlohmann::json& json_ref);
};
