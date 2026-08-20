/*---------------------------------------------------------*\
| nvrm_api.h                                                |
|                                                           |
|   Minimal NVIDIA Resource Manager API definitions for     |
|   issuing I2C transactions through /dev/nvidiactl on      |
|   Linux. Struct layouts and constants are the stable      |
|   kernel<->userspace ABI of the NVIDIA driver, taken      |
|   from NVIDIA/open-gpu-kernel-modules (MIT licensed):     |
|     kernel-open/common/inc/nv-ioctl-numbers.h             |
|     src/nvidia/arch/nvalloc/unix/include/nv_escape.h      |
|     src/common/sdk/nvidia/inc/nvos.h                      |
|     src/common/sdk/nvidia/inc/ctrl/ctrl402c.h             |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <stdint.h>

typedef uint8_t  NvU8;
typedef uint16_t NvU16;
typedef uint32_t NvU32;
typedef uint64_t NvU64;
typedef uint32_t NvHandle;
typedef uint32_t NvV32;
typedef NvU8     NvBool;
typedef NvU64    NvP64;      /* 64-bit pointer container */

/* ioctl plumbing (nv-ioctl-numbers.h) */
#define NV_IOCTL_MAGIC              'F'

/* escape codes (nv_escape.h) */
#define NV_ESC_RM_CONTROL           0x2A
#define NV_ESC_RM_ALLOC             0x2B
#define NV_ESC_RM_FREE              0x29
#define NV_ESC_CHECK_VERSION_STR    (200 + 10)   /* NV_IOCTL_BASE + 10 */

/* version handshake (nv-ioctl.h) */
#define NV_RM_API_VERSION_STRING_LENGTH 64
#define NV_RM_API_VERSION_CMD_RELAXED   '1'
#define NV_RM_API_VERSION_REPLY_RECOGNIZED 1

typedef struct
{
    NvU32 cmd;
    NvU32 reply;
    char  versionString[NV_RM_API_VERSION_STRING_LENGTH];
} nv_ioctl_rm_api_version_t;

/* object classes */
#define NV01_ROOT_CLIENT            0x00000041
#define NV01_DEVICE_0               0x00000080
#define NV20_SUBDEVICE_0            0x00002080
#define NV40_I2C                    0x0000402C

/* NVOS21_PARAMETERS - NV_ESC_RM_ALLOC (nvos.h) */
typedef struct
{
    NvHandle hRoot;
    NvHandle hObjectParent;
    NvHandle hObjectNew;
    NvV32    hClass;
    NvP64    pAllocParms __attribute__((aligned(8)));
    NvU32    paramsSize;
    NvV32    status;
} NVOS21_PARAMETERS;

/* NVOS54_PARAMETERS - NV_ESC_RM_CONTROL (nvos.h) */
typedef struct
{
    NvHandle hClient;
    NvHandle hObject;
    NvV32    cmd;
    NvU32    flags;
    NvP64    params __attribute__((aligned(8)));
    NvU32    paramsSize;
    NvV32    status;
} NVOS54_PARAMETERS;

/* NVOS00_PARAMETERS - NV_ESC_RM_FREE (nvos.h) */
typedef struct
{
    NvHandle hRoot;
    NvHandle hObjectParent;
    NvHandle hObjectOld;
    NvV32    status;
} NVOS00_PARAMETERS;

/* NV0080_ALLOC_PARAMETERS (cl0080.h) */
typedef struct
{
    NvU32    deviceId;
    NvHandle hClientShare;
    NvHandle hTargetClient;
    NvHandle hTargetDevice;
    NvV32    flags;
    NvU64    vaSpaceSize      __attribute__((aligned(8)));
    NvU64    vaStartInternal  __attribute__((aligned(8)));
    NvU64    vaLimitInternal  __attribute__((aligned(8)));
    NvV32    vaMode;
} NV0080_ALLOC_PARAMETERS;

/* NV2080_ALLOC_PARAMETERS (cl2080.h) */
typedef struct
{
    NvU32 subDeviceId;
} NV2080_ALLOC_PARAMETERS;

/* NV402C i2c transaction control (ctrl402c.h) */
#define NV402C_CTRL_CMD_I2C_TRANSACTION 0x402c0105

/* flags: SPEED_MODE is bitfield 4:1 */
#define NV402C_I2C_SPEED_100KHZ  (0x0 << 1)
#define NV402C_I2C_SPEED_200KHZ  (0x1 << 1)
#define NV402C_I2C_SPEED_400KHZ  (0x2 << 1)
#define NV402C_I2C_SPEED_33KHZ   (0x3 << 1)
#define NV402C_I2C_SPEED_10KHZ   (0x4 << 1)
#define NV402C_I2C_SPEED_3KHZ    (0x5 << 1)
#define NV402C_I2C_SPEED_300KHZ  (0x7 << 1)

typedef enum
{
    NV402C_TRANS_SMBUS_QUICK_RW       = 0,
    NV402C_TRANS_I2C_BYTE_RW          = 1,
    NV402C_TRANS_I2C_BLOCK_RW         = 2,
    NV402C_TRANS_I2C_BUFFER_RW        = 3,
    NV402C_TRANS_SMBUS_BYTE_RW        = 4,
    NV402C_TRANS_SMBUS_WORD_RW        = 5,
    NV402C_TRANS_SMBUS_BLOCK_RW       = 6,
    NV402C_TRANS_SMBUS_PROCESS_CALL   = 7,
    NV402C_TRANS_SMBUS_BLK_PROC_CALL  = 8,
    NV402C_TRANS_SMBUS_MULTIBYTE_RW   = 9,
} NV402C_TRANS_TYPE;

typedef struct
{
    NvBool bWrite;
} NV402C_DATA_SMBUS_QUICK_RW;

typedef struct
{
    NvBool bWrite;
    NvU8   message;
} NV402C_DATA_I2C_BYTE_RW;

typedef struct
{
    NvBool bWrite;
    NvU32  messageLength;
    NvP64  pMessage __attribute__((aligned(8)));
} NV402C_DATA_I2C_BLOCK_RW;

typedef struct
{
    NvBool bWrite;
    NvU8   registerAddress;
    NvU8   message;
} NV402C_DATA_SMBUS_BYTE_RW;

typedef struct
{
    NvBool bWrite;
    NvU8   registerAddress;
    NvU16  message;
} NV402C_DATA_SMBUS_WORD_RW;

typedef struct
{
    NvBool bWrite;
    NvU32  warFlags;
    NvU8   registerAddress;
    NvU32  messageLength;
    NvP64  pMessage __attribute__((aligned(8)));
} NV402C_DATA_I2C_BUFFER_RW;

typedef struct
{
    NvBool bWrite;
    NvU8   registerAddress;
    NvU32  messageLength;
    NvP64  pMessage __attribute__((aligned(8)));
} NV402C_DATA_SMBUS_BLOCK_RW;

/* Union padded to the exact ABI size (80 bytes; largest real member is
 * the EDID data variant). Total params struct must be 96 bytes with
 * transData at offset 16 - verified against the SDK headers. */
typedef union
{
    NV402C_DATA_SMBUS_QUICK_RW smbusQuickData;
    NV402C_DATA_I2C_BYTE_RW    i2cByteData;
    NV402C_DATA_I2C_BLOCK_RW   i2cBlockData;
    NV402C_DATA_SMBUS_BYTE_RW  smbusByteData;
    NV402C_DATA_SMBUS_WORD_RW  smbusWordData;
    NV402C_DATA_I2C_BUFFER_RW  i2cBufferData;
    NV402C_DATA_SMBUS_BLOCK_RW smbusBlockData;
    NvU8 _pad[80];
} NV402C_TRANS_DATA;

typedef struct
{
    NvU8              portId;
    NvU32             flags;
    NvU16             deviceAddress;
    NV402C_TRANS_TYPE transType;
    NV402C_TRANS_DATA transData __attribute__((aligned(8)));
} NV402C_CTRL_I2C_TRANSACTION_PARAMS;

static_assert(sizeof(NV402C_CTRL_I2C_TRANSACTION_PARAMS) == 96,
              "NV402C transaction params ABI size mismatch");
