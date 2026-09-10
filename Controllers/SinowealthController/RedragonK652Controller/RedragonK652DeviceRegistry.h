/*---------------------------------------------------------*\
| RedragonK652DeviceRegistry.h                              |
|                                                           |
|   Per-container ownership for the wired Redragon K652     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstddef>
#include <memory>
#include <string>

using RedragonK652DeviceClaim = std::shared_ptr<const std::string>;

class RedragonK652DeviceRegistry
{
public:
    static RedragonK652DeviceClaim Claim(const void* container_id, std::size_t container_id_size);
};
