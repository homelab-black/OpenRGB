/*---------------------------------------------------------*\
| JsonUtils.cpp                                             |
|                                                           |
|   JSON utility functions                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "JsonUtils.h"

bool JsonUtils::JsonParse(std::string json_string, nlohmann::json& json_ref)
{
    try
    {
        json_ref = nlohmann::json::parse(json_string);
        return(true);
    }
    catch(...)
    {
        return(false);
    }
}
