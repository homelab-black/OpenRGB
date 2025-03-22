/*---------------------------------------------------------*\
| main.cpp                                                  |
|                                                           |
|   Entry point for the OpenRGB application                 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "cli.h"
#include "ResourceManager.h"
#include "NetworkServer.h"
#include "LogManager.h"
#include "startup.h"

#ifdef _MACOSX_X86_X64
#include "macUSPCIOAccess.h"
io_connect_t macUSPCIO_driver_connection;
#endif

#ifdef __APPLE__
#include "macutils.h"
#endif

using namespace std::chrono_literals;

void WaitWhileServerOnline(NetworkServer* srv)
{
    while (srv->GetOnline())
    {
        std::this_thread::sleep_for(1s);
    };
}

/******************************************************************************************\
*                                                                                          *
*   main                                                                                   *
*                                                                                          *
*       some housekeeping and call startup()                                               *
*                                                                                          *
\******************************************************************************************/
int main(int argc, char* argv[])
{
    /*---------------------------------------------------------*\
    | Mac x86/x64 only - Install SMBus Driver macUSPCIO         |
    \*---------------------------------------------------------*/
#ifdef _MACOSX_X86_X64
    InitMacUSPCIODriver();
#endif

    unsigned int ret_flags = cli_pre_detection(argc, argv);
    ResourceManager::get()->Initialize(
        !(ret_flags & RET_FLAG_NO_AUTO_CONNECT),
        !(ret_flags & RET_FLAG_NO_DETECT),
        ret_flags & RET_FLAG_START_SERVER,
        ret_flags & RET_FLAG_CLI_POST_DETECTION);

    int exitval = startup(argc, argv, ret_flags);

    if((ret_flags & RET_FLAG_START_SERVER) && !(ret_flags & RET_FLAG_START_GUI))
    {
        NetworkServer* server = ResourceManager::get()->GetServer();
        if(server)
        {
            WaitWhileServerOnline(server);
        }
    }
    ResourceManager::get()->Cleanup();
    LOG_TRACE("OpenRGB finishing with exit code %d", exitval);
#ifdef _MACOSX_X86_X64
    CloseMacUSPCIODriver();
#endif
    return exitval;
}
