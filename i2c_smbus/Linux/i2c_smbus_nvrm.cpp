/*---------------------------------------------------------*\
| i2c_smbus_nvrm.cpp                                        |
|                                                           |
|   NVIDIA GPU i2c driver for Linux using the Resource      |
|   Manager API (/dev/nvidiactl).                           |
|                                                           |
|   The kernel's /dev/i2c-* adapters for NVIDIA GPUs run    |
|   at a fixed 100 kHz, so i2c devices that require slower  |
|   clocks (Zotac Spectra RGB MCUs want 10 kHz) never       |
|   respond through them. The RM API accepts a per-         |
|   transaction speed flag - the same mechanism NvAPI       |
|   exposes on Windows.                                     |
|                                                           |
|   To avoid double-detecting devices that are already      |
|   reachable through /dev/i2c-*, this bus only performs    |
|   transfers after a detector has requested a reduced      |
|   speed via i2c_set_max_speed(); at default speed all     |
|   transfers report failure immediately.                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <sys/ioctl.h>
#include "LogManager.h"
#include "i2c_smbus_nvrm.h"

#define NVRM_H_DEVICE 0x5F524742  /* arbitrary client-chosen handles "_RGB" */
#define NVRM_H_SUBDEV 0x5F524743
#define NVRM_H_I2C    0x5F524744

/* NvAPI on Windows always uses GPU port 1 for RGB controllers; the RM
 * port numbering is the same. */
#define NVRM_RGB_PORT_ID 1

i2c_smbus_nvrm::i2c_smbus_nvrm()
{
    ctl_fd      = -1;
    dev_fd      = -1;
    h_client    = 0;
    speed_flags = NV402C_I2C_SPEED_100KHZ;
    speed_set   = false;
}

i2c_smbus_nvrm::~i2c_smbus_nvrm()
{
    if(h_client && ctl_fd >= 0)
    {
        NVOS00_PARAMETERS f;
        memset(&f, 0, sizeof(f));
        f.hRoot         = h_client;
        f.hObjectParent = h_client;
        f.hObjectOld    = h_client;
        nv_ioctl(ctl_fd, NV_ESC_RM_FREE, &f, sizeof(f));
    }
    if(dev_fd >= 0)
    {
        close(dev_fd);
    }
    if(ctl_fd >= 0)
    {
        close(ctl_fd);
    }
}

int i2c_smbus_nvrm::nv_ioctl(int fd, unsigned cmd, void* data, size_t size)
{
    return ioctl(fd, _IOC(_IOC_READ|_IOC_WRITE, NV_IOCTL_MAGIC, cmd, size), data);
}

NvU32 i2c_smbus_nvrm::rm_alloc(NvHandle parent, NvHandle hnew, NvU32 hclass,
                               void* parms, NvU32 parms_size, NvHandle* out)
{
    NVOS21_PARAMETERS p;
    memset(&p, 0, sizeof(p));
    p.hRoot         = h_client;
    p.hObjectParent = parent;
    p.hObjectNew    = hnew;
    p.hClass        = hclass;
    p.pAllocParms   = (NvP64)(uintptr_t)parms;
    p.paramsSize    = parms_size;
    if(nv_ioctl(ctl_fd, NV_ESC_RM_ALLOC, &p, sizeof(p)) < 0)
    {
        return 0xFFFFFFFF;
    }
    if(p.status == 0 && out)
    {
        *out = p.hObjectNew;
    }
    return p.status;
}

NvU32 i2c_smbus_nvrm::rm_i2c(NV402C_CTRL_I2C_TRANSACTION_PARAMS* params)
{
    NVOS54_PARAMETERS p;
    memset(&p, 0, sizeof(p));
    p.hClient    = h_client;
    p.hObject    = NVRM_H_I2C;
    p.cmd        = NV402C_CTRL_CMD_I2C_TRANSACTION;
    p.params     = (NvP64)(uintptr_t)params;
    p.paramsSize = sizeof(*params);
    if(nv_ioctl(ctl_fd, NV_ESC_RM_CONTROL, &p, sizeof(p)) < 0)
    {
        return 0xFFFFFFFF;
    }
    return p.status;
}

bool i2c_smbus_nvrm::open_gpu(int minor)
{
    char devpath[32];

    ctl_fd = open("/dev/nvidiactl", O_RDWR);
    if(ctl_fd < 0)
    {
        return false;
    }

    snprintf(devpath, sizeof(devpath), "/dev/nvidia%d", minor);
    dev_fd = open(devpath, O_RDWR);
    if(dev_fd < 0)
    {
        return false;
    }

    /*-----------------------------------------------------------------*\
    | Version handshake. RELAXED matching accepts any same-major driver |
    \*-----------------------------------------------------------------*/
    nv_ioctl_rm_api_version_t v;
    memset(&v, 0, sizeof(v));
    v.cmd = NV_RM_API_VERSION_CMD_RELAXED;

    FILE* f = fopen("/proc/driver/nvidia/version", "r");
    if(f)
    {
        char line[256];
        if(fgets(line, sizeof(line), f))
        {
            for(char* p = line; *p; )
            {
                if(isdigit((unsigned char)*p))
                {
                    int dots = 0;
                    char* q = p;
                    while(isdigit((unsigned char)*q) || *q == '.')
                    {
                        if(*q == '.') dots++;
                        q++;
                    }
                    if(dots >= 1 && q - p < (long)sizeof(v.versionString))
                    {
                        memcpy(v.versionString, p, q - p);
                        break;
                    }
                    p = q;
                }
                else
                {
                    p++;
                }
            }
        }
        fclose(f);
    }

    if(nv_ioctl(ctl_fd, NV_ESC_CHECK_VERSION_STR, &v, sizeof(v)) < 0
       || v.reply != NV_RM_API_VERSION_REPLY_RECOGNIZED)
    {
        LOG_INFO("[i2c_smbus_nvrm] RM API version handshake failed");
        return false;
    }

    if(rm_alloc(0, 0, NV01_ROOT_CLIENT, NULL, 0, &h_client) != 0)
    {
        return false;
    }

    NV0080_ALLOC_PARAMETERS dp;
    memset(&dp, 0, sizeof(dp));
    dp.deviceId     = minor;
    dp.hClientShare = h_client;
    if(rm_alloc(h_client, NVRM_H_DEVICE, NV01_DEVICE_0, &dp, sizeof(dp), NULL) != 0)
    {
        return false;
    }

    NV2080_ALLOC_PARAMETERS sp;
    memset(&sp, 0, sizeof(sp));
    if(rm_alloc(NVRM_H_DEVICE, NVRM_H_SUBDEV, NV20_SUBDEVICE_0, &sp, sizeof(sp), NULL) != 0)
    {
        return false;
    }

    if(rm_alloc(NVRM_H_SUBDEV, NVRM_H_I2C, NV40_I2C, NULL, 0, NULL) != 0)
    {
        return false;
    }

    return true;
}

void i2c_smbus_nvrm::i2c_set_max_speed(int hz)
{
    if(hz <= 0 || hz >= 100000)
    {
        speed_flags = NV402C_I2C_SPEED_100KHZ;
        speed_set   = false;
    }
    else if(hz <= 3000)
    {
        speed_flags = NV402C_I2C_SPEED_3KHZ;
        speed_set   = true;
    }
    else if(hz <= 10000)
    {
        speed_flags = NV402C_I2C_SPEED_10KHZ;
        speed_set   = true;
    }
    else
    {
        speed_flags = NV402C_I2C_SPEED_33KHZ;
        speed_set   = true;
    }
}

s32 i2c_smbus_nvrm::i2c_smbus_xfer(u8 addr, char read_write, u8 command, int size, i2c_smbus_data* data)
{
    /*-----------------------------------------------------------------*\
    | Refuse transfers at default speed: those devices are reachable    |
    | through /dev/i2c-* already and would be detected twice.           |
    \*-----------------------------------------------------------------*/
    if(!speed_set)
    {
        return -1;
    }

    NV402C_CTRL_I2C_TRANSACTION_PARAMS t;
    memset(&t, 0, sizeof(t));
    t.portId        = NVRM_RGB_PORT_ID;
    t.flags         = speed_flags;
    t.deviceAddress = (NvU16)(addr << 1);

    bool is_write = (read_write == I2C_SMBUS_WRITE);

    switch(size)
    {
        case I2C_SMBUS_QUICK:
            t.transType = NV402C_TRANS_SMBUS_QUICK_RW;
            t.transData.smbusQuickData.bWrite = is_write;
            break;

        case I2C_SMBUS_BYTE:
            t.transType = NV402C_TRANS_I2C_BYTE_RW;
            t.transData.i2cByteData.bWrite  = is_write;
            t.transData.i2cByteData.message = is_write ? command : 0;
            break;

        case I2C_SMBUS_BYTE_DATA:
            t.transType = NV402C_TRANS_SMBUS_BYTE_RW;
            t.transData.smbusByteData.bWrite          = is_write;
            t.transData.smbusByteData.registerAddress = command;
            t.transData.smbusByteData.message         = is_write ? data->byte : 0;
            break;

        case I2C_SMBUS_WORD_DATA:
            t.transType = NV402C_TRANS_SMBUS_WORD_RW;
            t.transData.smbusWordData.bWrite          = is_write;
            t.transData.smbusWordData.registerAddress = command;
            t.transData.smbusWordData.message         = is_write ? data->word : 0;
            break;

        case I2C_SMBUS_BLOCK_DATA:
            t.transType = NV402C_TRANS_SMBUS_BLOCK_RW;
            t.transData.smbusBlockData.bWrite          = is_write;
            t.transData.smbusBlockData.registerAddress = command;
            t.transData.smbusBlockData.messageLength   = data->block[0];
            t.transData.smbusBlockData.pMessage        = (NvP64)(uintptr_t)&data->block[1];
            break;

        case I2C_SMBUS_I2C_BLOCK_DATA:
            /* i2c "block" - register address, repeated start, no count byte */
            t.transType = NV402C_TRANS_I2C_BUFFER_RW;
            t.transData.i2cBufferData.bWrite          = is_write;
            t.transData.i2cBufferData.registerAddress = command;
            t.transData.i2cBufferData.messageLength   = data->block[0];
            t.transData.i2cBufferData.pMessage        = (NvP64)(uintptr_t)&data->block[1];
            break;

        default:
            return -1;
    }

    NvU32 status = rm_i2c(&t);
    if(status != 0)
    {
        LOG_TRACE("[i2c_smbus_nvrm] xfer addr=0x%02X size=%d rw=%d speed_flags=0x%X -> RM status 0x%08X",
                  addr, size, read_write, speed_flags, status);
        return -1;
    }

    if(!is_write)
    {
        switch(size)
        {
            case I2C_SMBUS_BYTE:
                data->byte = t.transData.i2cByteData.message;
                break;
            case I2C_SMBUS_BYTE_DATA:
                data->byte = t.transData.smbusByteData.message;
                break;
            case I2C_SMBUS_WORD_DATA:
                data->word = t.transData.smbusWordData.message;
                break;
            case I2C_SMBUS_BLOCK_DATA:
                data->block[0] = (u8)t.transData.smbusBlockData.messageLength;
                break;
            case I2C_SMBUS_I2C_BLOCK_DATA:
                data->block[0] = (u8)t.transData.i2cBufferData.messageLength;
                break;
        }
    }

    return 0;
}

s32 i2c_smbus_nvrm::i2c_xfer(u8 addr, char read_write, int* size, u8* data)
{
    if(!speed_set)
    {
        return -1;
    }

    NV402C_CTRL_I2C_TRANSACTION_PARAMS t;
    memset(&t, 0, sizeof(t));
    t.portId        = NVRM_RGB_PORT_ID;
    t.flags         = speed_flags;
    t.deviceAddress = (NvU16)(addr << 1);
    t.transType     = NV402C_TRANS_I2C_BLOCK_RW;
    t.transData.i2cBlockData.bWrite        = (read_write == I2C_SMBUS_WRITE);
    t.transData.i2cBlockData.messageLength = *size;
    t.transData.i2cBlockData.pMessage      = (NvP64)(uintptr_t)data;

    NvU32 status = rm_i2c(&t);
    if(status != 0)
    {
        return -1;
    }

    if(read_write == I2C_SMBUS_READ)
    {
        *size = t.transData.i2cBlockData.messageLength;
    }

    return 0;
}

/******************************************************************************************\
*                                                                                          *
*   i2c_smbus_nvrm_detect                                                                  *
*                                                                                          *
*       Registers one RM-API i2c bus per NVIDIA GPU, carrying the GPU's PCI IDs so         *
*       that REGISTER_I2C_PCI_DETECTOR entries match against it.                           *
*                                                                                          *
\******************************************************************************************/

#include "DetectionManager.h"

bool i2c_smbus_nvrm_detect()
{
    DIR* dir = opendir("/proc/driver/nvidia/gpus");
    if(dir == NULL)
    {
        return(false);
    }

    struct dirent* ent;
    while((ent = readdir(dir)) != NULL)
    {
        if(ent->d_name[0] == '.')
        {
            continue;
        }

        /*-------------------------------------------------------------*\
        | Read PCI IDs from sysfs and the device minor from procfs      |
        \*-------------------------------------------------------------*/
        char path[512];
        char buff[128];
        unsigned int pci_vendor = 0, pci_device = 0;
        unsigned int pci_subsystem_vendor = 0, pci_subsystem_device = 0;
        int minor = -1;

        snprintf(path, sizeof(path), "/proc/driver/nvidia/gpus/%s/information", ent->d_name);
        FILE* f = fopen(path, "r");
        if(f == NULL)
        {
            continue;
        }
        while(fgets(buff, sizeof(buff), f))
        {
            sscanf(buff, "Device Minor: %d", &minor);
        }
        fclose(f);
        if(minor < 0)
        {
            continue;
        }

        const struct { const char* file; unsigned int* val; } ids[] =
        {
            { "vendor",           &pci_vendor           },
            { "device",           &pci_device           },
            { "subsystem_vendor", &pci_subsystem_vendor },
            { "subsystem_device", &pci_subsystem_device },
        };
        for(unsigned int i = 0; i < 4; i++)
        {
            snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/%s", ent->d_name, ids[i].file);
            f = fopen(path, "r");
            if(f)
            {
                if(fgets(buff, sizeof(buff), f))
                {
                    *ids[i].val = strtoul(buff, NULL, 16);
                }
                fclose(f);
            }
        }

        i2c_smbus_nvrm* bus = new i2c_smbus_nvrm();
        if(!bus->open_gpu(minor))
        {
            delete bus;
            continue;
        }

        snprintf(bus->info.device_name, sizeof(bus->info.device_name),
                 "NVIDIA RM i2c port %d (GPU %s)", NVRM_RGB_PORT_ID, ent->d_name);
        bus->info.pci_vendor           = pci_vendor;
        bus->info.pci_device           = pci_device;
        bus->info.pci_subsystem_vendor = pci_subsystem_vendor;
        bus->info.pci_subsystem_device = pci_subsystem_device;
        bus->info.port_id              = NVRM_RGB_PORT_ID;
        bus->info.bus_id               = minor;
        DetectionManager::get()->RegisterI2CBus(bus);

        LOG_INFO("[i2c_smbus_nvrm] registered RM i2c bus for GPU %s (%04X:%04X %04X:%04X)",
                 ent->d_name, pci_vendor, pci_device,
                 pci_subsystem_vendor, pci_subsystem_device);
    }
    closedir(dir);

    return(true);
}

REGISTER_I2C_BUS_DETECTOR(i2c_smbus_nvrm_detect);
