/*---------------------------------------------------------*\
| i2c_smbus_nvrm.h                                          |
|                                                           |
|   NVIDIA GPU i2c driver for Linux using the Resource      |
|   Manager API (/dev/nvidiactl), which supports per-       |
|   transaction bus speed selection - required by slow      |
|   RGB controllers (e.g. Zotac Spectra at 10 kHz) that     |
|   the fixed-100kHz /dev/i2c-* adapters cannot reach.      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "i2c_smbus.h"
#include "nvrm_api.h"

class i2c_smbus_nvrm : public i2c_smbus_interface
{
public:
    i2c_smbus_nvrm();
    ~i2c_smbus_nvrm();

    bool open_gpu(int minor);

    s32 i2c_smbus_xfer(u8 addr, char read_write, u8 command, int size, i2c_smbus_data* data);
    s32 i2c_xfer(u8 addr, char read_write, int* size, u8* data);
    void i2c_set_max_speed(int hz);

private:
    int      ctl_fd;
    int      dev_fd;
    NvHandle h_client;
    NvU32    speed_flags;
    bool     speed_set;

    int   nv_ioctl(int fd, unsigned cmd, void* data, size_t size);
    NvU32 rm_alloc(NvHandle parent, NvHandle hnew, NvU32 hclass, void* parms, NvU32 parms_size, NvHandle* out);
    NvU32 rm_i2c(NV402C_CTRL_I2C_TRANSACTION_PARAMS* params);
};
