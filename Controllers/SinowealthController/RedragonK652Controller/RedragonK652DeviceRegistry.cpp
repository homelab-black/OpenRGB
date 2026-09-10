/*---------------------------------------------------------*\
| RedragonK652DeviceRegistry.cpp                            |
|                                                           |
|   Per-container ownership for the wired Redragon K652     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RedragonK652DeviceRegistry.h"

#include <mutex>
#include <set>

namespace
{
    struct RedragonK652RegistryState
    {
        std::mutex            mutex;
        std::set<std::string> claimed_containers;
    };

    std::shared_ptr<RedragonK652RegistryState> GetRedragonK652RegistryState()
    {
        static const std::shared_ptr<RedragonK652RegistryState> state = std::make_shared<RedragonK652RegistryState>();
        return(state);
    }
}

RedragonK652DeviceClaim RedragonK652DeviceRegistry::Claim(const void* container_id, std::size_t container_id_size)
{
    if(!container_id || container_id_size == 0)
    {
        return(RedragonK652DeviceClaim());
    }

    std::unique_ptr<std::string> key(new std::string(static_cast<const char*>(container_id), container_id_size));
    const std::shared_ptr<RedragonK652RegistryState> state = GetRedragonK652RegistryState();

    {
        const std::lock_guard<std::mutex> lock(state->mutex);

        if(!state->claimed_containers.insert(*key).second)
        {
            return(RedragonK652DeviceClaim());
        }
    }

    /*-----------------------------------------------------------------*\
    | Construct the lifetime token after releasing the registry mutex.  |
    | If shared_ptr allocation fails, its deleter can safely reacquire  |
    | the mutex and roll back the inserted claim.                        |
    \*-----------------------------------------------------------------*/
    return(RedragonK652DeviceClaim(key.release(), [state](const std::string* claimed_key)
    {
        const std::unique_ptr<const std::string> owned_key(claimed_key);
        const std::lock_guard<std::mutex> lock(state->mutex);
        state->claimed_containers.erase(*owned_key);
    }));
}
