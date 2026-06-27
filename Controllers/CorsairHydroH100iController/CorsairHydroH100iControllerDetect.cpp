/*---------------------------------------------------------*\
| CorsairHydroH100iControllerDetect.cpp                     |
|                                                           |
|   Detector for Corsair H100i Hydro Series cooler          |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          27 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <libusb.h>
#include "Detector.h"
#include "CorsairHydroH100iController.h"
#include "RGBController_CorsairHydroH100i.h"

/*-----------------------------------------------------*\
| Corsair vendor ID                                     |
\*-----------------------------------------------------*/
#define CORSAIR_VID                     0x1B1C

/*-----------------------------------------------------*\
| Corsair H100i product ID (legacy Coolit protocol)    |
\*-----------------------------------------------------*/
#define CORSAIR_H100I_PID               0x0C04

typedef struct
{
    unsigned short  usb_vid;
    unsigned short  usb_pid;
    unsigned char   usb_interface;
    const char *    name;
} corsair_hydro_h100i_device;

#define CORSAIR_H100I_NUM_DEVICES (sizeof(device_list) / sizeof(device_list[0]))

static const corsair_hydro_h100i_device device_list[] =
{
    /*-----------------------------------------------------------------------------------------------------*\
    | Coolers (H100i using legacy Coolit protocol)                                                        |
    \*-----------------------------------------------------------------------------------------------------*/
    { CORSAIR_VID,          CORSAIR_H100I_PID,          0,      "Corsair H100i"                         },
};

/******************************************************************************************\
*                                                                                          *
*   DetectCorsairHydroH100iControllers                                                     *
*                                                                                          *
*       Tests the USB address to see if a Corsair H100i RGB Cooler exists there.           *
*                                                                                          *
\******************************************************************************************/

void DetectCorsairHydroH100iControllers()
{
    libusb_init(NULL);

    #ifdef _WIN32
    libusb_set_option(NULL, LIBUSB_OPTION_USE_USBDK);
    #endif

    for(std::size_t device_idx = 0; device_idx < CORSAIR_H100I_NUM_DEVICES; device_idx++)
    {
        libusb_device_handle* dev = libusb_open_device_with_vid_pid(NULL, device_list[device_idx].usb_vid, device_list[device_idx].usb_pid);

        /*-----------------------------------------------------*\
        | Look for Corsair H100i RGB Cooler                     |
        \*-----------------------------------------------------*/
        if(dev)
        {
            libusb_detach_kernel_driver(dev, 0);
            libusb_claim_interface(dev, 0);

            CorsairHydroH100iController*     controller     = new CorsairHydroH100iController(dev, device_list[device_idx].name);
            RGBController_CorsairHydroH100i* rgb_controller = new RGBController_CorsairHydroH100i(controller);

            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
    }
}   /* DetectCorsairHydroH100iControllers() */

REGISTER_DETECTOR("Corsair H100i Series", DetectCorsairHydroH100iControllers);
/*---------------------------------------------------------------------------------------------------------*\
| Entries for dynamic UDEV rules                                                                            |
|                                                                                                           |
| DUMMY_DEVICE_DETECTOR("Corsair H100i Series", DetectCorsairHydroH100iControllers, 0x1B1C, 0x0C04 )      |
\*---------------------------------------------------------------------------------------------------------*/
