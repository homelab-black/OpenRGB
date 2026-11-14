/*---------------------------------------------------------*\
| SamsungSSDController_Linux.cpp                            |
|                                                           |
|   Driver for Samsung SSD heatsink LED (Linux)             |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/nvme_ioctl.h>
#include "SamsungSSDController_Windows_Linux.h"

#define NVME_ADMIN_SECURITY_SEND        0x81
#define NVME_TIMEOUT_MS                 15000

/*---------------------------------------------------------*\
| On Windows the command goes to LUN 0, which stornvme maps |
| to namespace 1. Linux uses the same namespace. This value |
| is not yet tested on Linux.                               |
\*---------------------------------------------------------*/
#define NVME_NAMESPACE_ID               1

SamsungSSDController::~SamsungSSDController()
{
    close(handle);
}

bool SamsungSSDController::SecuritySend(unsigned char* data, unsigned int size)
{
    struct nvme_admin_cmd command;

    memset(&command, 0, sizeof(command));

    command.opcode      = NVME_ADMIN_SECURITY_SEND;
    command.nsid        = NVME_NAMESPACE_ID;
    command.addr        = (uint64_t)(uintptr_t)data;
    command.data_len    = size;
    command.cdw10       = (SAMSUNG_SSD_SECURITY_PROTOCOL << 24) | (SAMSUNG_SSD_SPSP_LED << 8);
    command.cdw11       = size;
    command.timeout_ms  = NVME_TIMEOUT_MS;

    /*-----------------------------------------------------*\
    | The ioctl returns a negative value for a system error |
    | and a positive NVMe status for a command error        |
    \*-----------------------------------------------------*/
    return(ioctl(handle, NVME_IOCTL_ADMIN_CMD, &command) == 0);
}
