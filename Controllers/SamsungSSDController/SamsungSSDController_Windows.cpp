/*---------------------------------------------------------*\
| SamsungSSDController_Windows.cpp                          |
|                                                           |
|   Driver for Samsung SSD heatsink LED (Windows)           |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstddef>
#include <cstring>
#include <windows.h>
#include <winioctl.h>
#include <ntddscsi.h>
#include "SamsungSSDController_Windows_Linux.h"

/*---------------------------------------------------------*\
| SCSI SECURITY PROTOCOL OUT. The inbox stornvme driver     |
| translates it to an NVMe Security Send command.           |
\*---------------------------------------------------------*/
#define SCSI_SECURITY_PROTOCOL_OUT      0xB5
#define SCSI_SECURITY_CDB_LENGTH        12
#define SCSI_SENSE_LENGTH               32
#define SCSI_TIMEOUT_SECONDS            15

struct scsi_pass_through_direct_with_sense
{
    SCSI_PASS_THROUGH_DIRECT    spt;
    ULONG                       filler;
    UCHAR                       sense[SCSI_SENSE_LENGTH];
};

SamsungSSDController::~SamsungSSDController()
{
    CloseHandle(handle);
}

bool SamsungSSDController::SecuritySend(unsigned char* data, unsigned int size)
{
    scsi_pass_through_direct_with_sense command;

    memset(&command, 0, sizeof(command));

    command.spt.Length              = sizeof(SCSI_PASS_THROUGH_DIRECT);
    command.spt.CdbLength           = SCSI_SECURITY_CDB_LENGTH;
    command.spt.SenseInfoLength     = SCSI_SENSE_LENGTH;
    command.spt.DataIn              = SCSI_IOCTL_DATA_OUT;
    command.spt.DataTransferLength  = size;
    command.spt.TimeOutValue        = SCSI_TIMEOUT_SECONDS;
    command.spt.DataBuffer          = data;
    command.spt.SenseInfoOffset     = offsetof(scsi_pass_through_direct_with_sense, sense);

    /*-----------------------------------------------------*\
    | CDB: protocol, protocol specific (big endian), and    |
    | transfer length in bytes (big endian)                 |
    \*-----------------------------------------------------*/
    command.spt.Cdb[0]              = SCSI_SECURITY_PROTOCOL_OUT;
    command.spt.Cdb[1]              = SAMSUNG_SSD_SECURITY_PROTOCOL;
    command.spt.Cdb[2]              = (SAMSUNG_SSD_SPSP_LED >> 8) & 0xFF;
    command.spt.Cdb[3]              = SAMSUNG_SSD_SPSP_LED & 0xFF;
    command.spt.Cdb[6]              = (size >> 24) & 0xFF;
    command.spt.Cdb[7]              = (size >> 16) & 0xFF;
    command.spt.Cdb[8]              = (size >> 8)  & 0xFF;
    command.spt.Cdb[9]              = size & 0xFF;

    DWORD bytes_returned            = 0;

    BOOL result = DeviceIoControl(handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, &command, sizeof(command), &command, sizeof(command), &bytes_returned, NULL);

    return(result && (command.spt.ScsiStatus == 0));
}
