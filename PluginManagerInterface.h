/*---------------------------------------------------------*\
| PluginManagerInterface.h                                  |
|                                                           |
|   OpenRGB plugin manager interface                        |
|                                                           |
|   Adam Honse <calcprogrammer1@gmail.com>      Nov 18 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "nlohmann/json.hpp"

class PluginManagerInterface
{
public:
    virtual void LoadPlugins()                                  = 0;
    virtual void UnloadPlugins()                                = 0;

    virtual void OnProfileAboutToLoad()                         = 0;
    virtual void OnProfileLoad(nlohmann::json profile_data)     = 0;
    virtual nlohmann::json OnProfileSave()                      = 0;
};
