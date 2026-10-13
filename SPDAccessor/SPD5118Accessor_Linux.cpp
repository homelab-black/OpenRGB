/*---------------------------------------------------------*\
| SPD5118Accessor_Linux.cpp                                 |
|                                                           |
|   DDR5 SPD accessor implementation using spd5118 driver   |
|   on Linux                                                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "SPD5118Accessor_Linux.h"
#include "filesystem.h"

const char *SPD5118Accessor::SPD_SPD5118_PATH = "/sys/bus/i2c/drivers/spd5118/%u-%04x/eeprom";

SPD5118Accessor::SPD5118Accessor(i2c_smbus_interface *bus, uint8_t spd_addr)
  : DDR5Accessor(bus, spd_addr), bytes_read(0), read_attempted(false)
{
    // Unreadable offsets must never expose uninitialized memory.
    memset(dump, 0xFF, sizeof(dump));
}

SPD5118Accessor::~SPD5118Accessor()
{
}

bool SPD5118Accessor::isAvailable(i2c_smbus_interface *bus, uint8_t spd_addr)
{
    int size = snprintf(nullptr, 0, SPD_SPD5118_PATH, bus->info.bus_id, spd_addr);
    char *path = new char[size+1];
    snprintf(path, size+1, SPD_SPD5118_PATH, bus->info.bus_id, spd_addr);
    bool result = std::filesystem::exists(path);
    delete[] path;
    return result;
}

SPDAccessor *SPD5118Accessor::copy()
{
    SPD5118Accessor *access = new SPD5118Accessor(bus, address);
    memcpy(access->dump, this->dump, sizeof(this->dump));
    access->bytes_read = this->bytes_read;
    access->read_attempted = this->read_attempted;
    return access;
}

uint8_t SPD5118Accessor::at(uint16_t addr)
{
    if(!read_attempted)
    {
        readEEPROM();
    }
    // Prevent indexing out of bounds
    // Some Linux spd5118 sysfs interfaces fail when reading later SPD pages.
    // Return an explicit unavailable value beyond the prefix read successfully.
    if(addr >= sizeof(dump) || addr >= bytes_read)
    {
        return 0xFF;
    }

    return dump[addr];
}

void SPD5118Accessor::readEEPROM()
{
    if(read_attempted)
    {
        return;
    }

    read_attempted = true;
    bytes_read = 0;
    memset(dump, 0xFF, sizeof(dump));

    int path_size = snprintf(nullptr, 0, SPD_SPD5118_PATH, bus->info.bus_id, address);
    if(path_size < 0)
    {
        return;
    }

    char *filename = new char[(std::size_t)path_size + 1];
    snprintf(filename, (std::size_t)path_size + 1,
             SPD_SPD5118_PATH, bus->info.bus_id, address);

    int fd = ::open(filename, O_RDONLY | O_CLOEXEC);
    delete[] filename;
    if(fd < 0)
    {
        return;
    }

    // Read in SPD5118 page-bounded chunks; later pages may be inaccessible.
    constexpr std::size_t page_size = 128;
    while(bytes_read < sizeof(dump))
    {
        const std::size_t page_remaining = page_size - (bytes_read % page_size);
        const std::size_t remaining = sizeof(dump) - bytes_read;
        const std::size_t request = remaining < page_remaining ? remaining : page_remaining;

        ssize_t result;
        do
        {
            result = ::pread(fd, dump + bytes_read, request, (off_t)bytes_read);
        }
        while(result < 0 && errno == EINTR);

        if(result <= 0)
        {
            break;
        }

        bytes_read += (std::size_t)result;
    }

    ::close(fd);
}
