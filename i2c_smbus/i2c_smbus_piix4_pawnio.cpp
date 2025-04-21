/*---------------------------------------------------------*\
| i2c_smbus_piix4_pawnio.cpp                                |
|                                                           |
|   PawnIO PIIX4 SMBUS driver for Windows                   |
|                                                           |
|   Stephen Horvath (Steve-Tech)                21 Apr 2025 |
|   Based on original OpenRGB PIIX4 source code             |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "i2c_smbus_piix4_pawnio.h"
#include "LogManager.h"
#ifdef _WIN32
#include "PawnIOLib.h"
#include "wmi.h"
#endif
#include "ResourceManager.h"
#include "SettingsManager.h"

i2c_smbus_piix4_pawnio::i2c_smbus_piix4_pawnio(void* handle, int port)
{
#ifdef _WIN32
    json drivers_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("Drivers");

    bool shared_smbus_access = true;
    if(drivers_settings.contains("shared_smbus_access"))
    {
        shared_smbus_access = drivers_settings["shared_smbus_access"].get<bool>();
    }
    if(shared_smbus_access)
    {
        global_smbus_access_handle = CreateMutexA(NULL, FALSE, GLOBAL_SMBUS_MUTEX_NAME);
    }

    pawnio_handle = (HANDLE)handle;
    this->port = port;

    using_handle++;
#endif
}

i2c_smbus_piix4_pawnio::~i2c_smbus_piix4_pawnio()
{
#ifdef _WIN32
    if(global_smbus_access_handle != NULL)
    {
        CloseHandle(global_smbus_access_handle);
    }

    if(--using_handle == 0 && pawnio_close(pawnio_handle)) {
        LOG_ERROR("PawnIO failed to close");
    }
#endif
}

s32 i2c_smbus_piix4_pawnio::piix4_pawnio_port_sel()
{
#ifdef _WIN32
    if(port == current_port) {
        return 0;
    }

    const SIZE_T in_size = 1;
    ULONG64 in[in_size] = {(unsigned)port};
    const SIZE_T out_size = 1;
    ULONG64 out[out_size];
    SIZE_T return_size;
    int status = pawnio_execute(pawnio_handle, "ioctl_piix4_port_sel", in, in_size, out, 1, &return_size);
    return status ? -EIO : 0;
#endif
}

s32 i2c_smbus_piix4_pawnio::piix4_pawnio_read(u8 addr, char /*read_write*/, u8 command, int size, i2c_smbus_data* data)
{
#ifdef _WIN32
    if(piix4_pawnio_port_sel())
    {
        return -EIO;
    }

    switch(size)
    {
    case I2C_SMBUS_BYTE:
    {
        const SIZE_T in_size = 1;
        ULONG64 in[in_size] = {addr};
        const SIZE_T out_size = 1;
        ULONG64 out[out_size];
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_read_byte", in, in_size, out, out_size, &return_size);
        data->byte = (u8)out[0];
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_BYTE_DATA:
    {
        const SIZE_T in_size = 2;
        ULONG64 in[in_size] = {addr, command};
        const SIZE_T out_size = 1;
        ULONG64 out[out_size];
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_read_byte_data", in, in_size, out, out_size, &return_size);
        data->byte = (u8)out[0];
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_WORD_DATA:
    {
        const SIZE_T in_size = 2;
        ULONG64 in[in_size] = {addr, command};
        const SIZE_T out_size = 1;
        ULONG64 out[out_size];
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_read_word_data", in, in_size, out, out_size, &return_size);
        data->word = (u16)out[0];
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_BLOCK_DATA:
    {
        const SIZE_T in_size = 2;
        ULONG64 in[in_size] = {addr, command};
        // SMBus' max blocks is 32, I know it's supposed to be bytes, but Pawn only deals with 64bit cells
        // The first cell is also the length
        const SIZE_T out_size = 1 + I2C_SMBUS_BLOCK_MAX;
        ULONG64 out[out_size];
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_read_block_data", in, in_size, out, out_size, &return_size);
        if(status)
        {
            return -EIO;
        }
        if(out[0] == 0 || out[0] > I2C_SMBUS_BLOCK_MAX)
        {
            return -EPROTO;
        }
        for (int i = 0; i <= out[0]; i++)
        {
            data->block[i] = (u8)out[i];
        }
        return 0;
    }
    default:
        return -EOPNOTSUPP;
    }
#endif
}

s32 i2c_smbus_piix4_pawnio::piix4_pawnio_write(u8 addr, char read_write, u8 command, int size, i2c_smbus_data* data)
{
#ifdef _WIN32
    if(piix4_pawnio_port_sel())
    {
        return -EIO;
    }

    switch(size)
    {
    case I2C_SMBUS_QUICK:
    {
        const SIZE_T in_size = 2;
        ULONG64 in[in_size] = {addr, (u8)read_write};
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_write_quick", in, in_size, NULL, 0, &return_size);
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_BYTE:
    {
        const SIZE_T in_size = 2;
        ULONG64 in[in_size] = {addr, data->byte};
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_write_byte", in, in_size, NULL, 0, &return_size);
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_BYTE_DATA:
    {
        const SIZE_T in_size = 3;
        ULONG64 in[in_size] = {addr, command, data->byte};
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_write_byte_data", in, in_size, NULL, 0, &return_size);
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_WORD_DATA:
    {
        const SIZE_T in_size = 3;
        ULONG64 in[in_size] = {addr, command, data->word};
        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_write_word_data", in, in_size, NULL, 0, &return_size);
        return status ? -EIO : 0;
    }
    case I2C_SMBUS_BLOCK_DATA:
    {

        int len = data->block[0];
        if(len == 0 || len > I2C_SMBUS_BLOCK_MAX)
        {
            return -EINVAL;
        }
        const SIZE_T in_size = 2 + len;
        ULONG64 in[2 + I2C_SMBUS_BLOCK_MAX] = {addr, command};

        for (int i = 0; i <= len; i++)
        {
            in[2+i] = data->block[i];
        }

        SIZE_T return_size;
        int status = pawnio_execute(pawnio_handle, "ioctl_piix4_write_block_data", in, in_size, NULL, 0, &return_size);
        return status ? -EIO : 0;
    }
    default:
        return -EOPNOTSUPP;
    }
#endif
}

s32 i2c_smbus_piix4_pawnio::i2c_smbus_xfer(u8 addr, char read_write, u8 command, int size, i2c_smbus_data* data)
{
#ifdef _WIN32
    if(global_smbus_access_handle != NULL)
    {
        WaitForSingleObject(global_smbus_access_handle, INFINITE);
    }

    int status = -ENXIO;

    if(read_write && size != I2C_SMBUS_QUICK) {
        status = piix4_pawnio_read(addr, read_write, command, size, data);
    }
    else
    {
        status = piix4_pawnio_write(addr, read_write, command, size, data);
    }

    if(global_smbus_access_handle != NULL)
    {
        ReleaseMutex(global_smbus_access_handle);
    }

    return status;
#endif
}

s32 i2c_smbus_piix4_pawnio::i2c_xfer(u8 /*addr*/, char /*read_write*/, int* /*size*/, u8* /*data*/)
{
    return -1;
}

int i2c_smbus_piix4_pawnio::using_handle = 0;
int i2c_smbus_piix4_pawnio::current_port = -1;

#ifdef _WIN32
HRESULT start_pawnio(PHANDLE phandle) {
    HRESULT status;

    status = pawnio_open(phandle);
    if(status)
    {
        if(E_ACCESSDENIED)
        {
            LOG_ERROR("Permission Denied, PawnIO piix4 I2C bus detection aborted");
        }
        else
        {
            LOG_ERROR("Could not open PawnIO, PawnIO piix4 I2C bus detection aborted");
        }
        return status;
    }

    HANDLE handle = *phandle;

    // Get the path of the executable
    char exePath[MAX_PATH];
    if(!GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
        LOG_ERROR("Failed to get executable path, PawnIO piix4 I2C bus detection aborted");
        return E_FAIL;
    }

    // Construct the path to I2cPIIX4.bin in the executable's directory
    std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
    std::filesystem::path filePath = exeDir / "I2cPIIX4.bin";

    // Check if the file exists
    if(!std::filesystem::exists(filePath)) {
        LOG_ERROR("Failed to find I2cPIIX4.bin in the executable's directory, PawnIO piix4 I2C bus detection aborted");
        return E_FAIL;
    }

    // Open the file and read its contents
    std::ifstream file(filePath, std::ios::binary);
    if(!file.is_open()) {
        LOG_ERROR("Failed to open I2cPIIX4.bin, PawnIO piix4 I2C bus detection aborted");
        return E_FAIL;
    }

    std::vector<char> blob((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Load the file into PawnIO
    status = pawnio_load(handle, reinterpret_cast<const UCHAR*>(blob.data()), blob.size());
    if(status)
    {
        LOG_ERROR("Failed to load I2cPIIX4.bin, PawnIO piix4 I2C bus detection aborted");
        return status;
    }

    return S_OK;
}

bool i2c_smbus_piix4_pawnio_detect()
{
    ULONG dll_version;
    if(pawnio_version(&dll_version))
    {
        LOG_INFO("PawnIO is not loaded, PawnIO piix4 I2C bus detection aborted");
        return(false);
    }

    i2c_smbus_interface * bus;
    HRESULT hres;
    Wmi wmi;
    HANDLE pawnio_handle;
    bool pawnio_was_init = false;

    // Query WMI for Win32_PnPSignedDriver entries with names matching "SMBUS" or "SM BUS"
    // These devices may be browsed under Device Manager -> System Devices
    std::vector<QueryObj> q_res_PnPSignedDriver;
    hres = wmi.query("SELECT * FROM Win32_PnPSignedDriver WHERE Description LIKE '%SMBUS%' OR Description LIKE '%SM BUS%'", q_res_PnPSignedDriver);

    if(hres)
    {
        LOG_INFO("WMI query failed, PawnIO piix4 I2C bus detection aborted");
        return(false);
    }

    // For each detected SMBus adapter, try enumerating it as either AMD or Intel
    for (QueryObj &i : q_res_PnPSignedDriver)
    {
        // AMD SMBus adapters use the PIIX4 driver
        if(i["Manufacturer"].find("Advanced Micro Devices, Inc") != std::string::npos)
        {
            std::string pnp_str = i["DeviceID"];

            std::size_t ven_loc = pnp_str.find("VEN_");
            std::size_t dev_loc = pnp_str.find("DEV_");
            std::size_t sub_loc = pnp_str.find("SUBSYS_");

            std::string ven_str = pnp_str.substr(ven_loc + 4, 4);
            std::string dev_str = pnp_str.substr(dev_loc + 4, 4);
            std::string sbv_str = pnp_str.substr(sub_loc + 11, 4);
            std::string sbd_str = pnp_str.substr(sub_loc + 7, 4);

            int ven_id = (int)std::stoul(ven_str, nullptr, 16);
            int dev_id = (int)std::stoul(dev_str, nullptr, 16);
            int sbv_id = (int)std::stoul(sbv_str, nullptr, 16);
            int sbd_id = (int)std::stoul(sbd_str, nullptr, 16);

            if(!pawnio_was_init && start_pawnio(&pawnio_handle) == S_OK)
            {
                i2c_smbus_piix4_pawnio::using_handle = 0;
                i2c_smbus_piix4_pawnio::current_port = -1;
                pawnio_was_init = true;
            }

            bus                         = new i2c_smbus_piix4_pawnio(pawnio_handle, 0);
            bus->pci_vendor             = ven_id;
            bus->pci_device             = dev_id;
            bus->pci_subsystem_vendor   = sbv_id;
            bus->pci_subsystem_device   = sbd_id;
            strcpy(bus->device_name, i["Description"].c_str());
            strcat(bus->device_name, " port 0");
            ResourceManager::get()->RegisterI2CBus(bus);

            bus                         = new i2c_smbus_piix4_pawnio(pawnio_handle, 1);
            bus->pci_vendor             = ven_id;
            bus->pci_device             = dev_id;
            bus->pci_subsystem_vendor   = sbv_id;
            bus->pci_subsystem_device   = sbd_id;
            strcpy(bus->device_name, i["Description"].c_str());
            strcat(bus->device_name, " port 1");
            ResourceManager::get()->RegisterI2CBus(bus);
        }
    }

    return(true);
}
#endif

REGISTER_I2C_BUS_DETECTOR(i2c_smbus_piix4_pawnio_detect);
