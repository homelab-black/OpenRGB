/*---------------------------------------------------------*\
| service.cpp                                               |
|                                                           |
|   Entry point for the OpenRGB application on Windows      |
|   Differentiate between service and application startup   |
|   Housekeeping for Windows service management             |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <thread>

#include "cli.h"
#include "startup.h"
#include "LogManager.h"
#include "NetworkServer.h"
#include "ResourceManager.h"

using namespace std::chrono_literals;

static int common_main(int argc, char* argv[]);

static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
static void ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

static char                  service_name[]           = "OpenRGB";
static SERVICE_TABLE_ENTRY   service_dispatch_table[] = { { service_name, ServiceMain }, { NULL, NULL } };
static DWORD                 service_checkpoint       = 1;
static SERVICE_STATUS_HANDLE service_status_handle;
static SERVICE_STATUS        service_status;

static bool                  started_as_service;
static volatile bool         service_stop_requested;
static bool                  have_console;

static std::mutex            service_status_mutex;

// detection progress goes from 0 to 100 twice, passing an estimate based on that confuses the service manager
#define PROGRESS_HACK
#ifdef PROGRESS_HACK
static int                   detection_pass;
static unsigned int          lastpercent = 101;
#endif

// report detection progress
static void ServiceStartupProgress(void*)
{
    unsigned int percent = ResourceManager::get()->GetDetectionPercent();
    unsigned int estimate;

    percent = std::clamp(percent, 0u, 100u);
#ifdef PROGRESS_HACK
    if(lastpercent > percent)
    {
        detection_pass += 1;
    }
    lastpercent = percent;
    switch(detection_pass)
    {
    case 0:
        percent = 0;
        break;
    case 1:
        percent = percent * 4 / 5;
        break;
    case 2:
        percent = percent / 5 + 80;
        break;
    default:
        percent = 100;
    }
#endif
    estimate = (100 - percent) / 5 + 10;
    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, estimate * 1000);
}

/*--------------------------------------------------------------------------------*\
|                                                                                  |
| As of writing this (24H2) there are 7 possible service states:                   |
|  SERVICE_START_PENDING, SERVICE_RUNNING, SERVICE_STOP_PENDING, SERVICE_STOPPED,  |
|  SERVICE_PAUSE_PENDING, SERVICE_PAUSED, SERVICE_CONTINUE_PENDING                 |
|                                                                                  |
| We don't accept pause requests, so only start_pending through stopped are        |
| possible.                                                                        |
|                                                                                  |
| Control requests can arrive asynchronously so we take some precautions here      |
| not to report an out of order status.                                            |
|                                                                                  |
\*--------------------------------------------------------------------------------*/

static void ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD last_state = SERVICE_START_PENDING;
    LOG_TRACE("ReportServiceStatus(%lu, %lu, %lu), PID %lu Thread %lu",
            dwCurrentState, dwWin32ExitCode, dwWaitHint, GetCurrentProcessId(), GetCurrentThreadId());
    service_status_mutex.lock();

    switch(last_state)
    {
    case SERVICE_RUNNING:
        // don't go back to SERVICE_START_PENDING
        if(dwCurrentState == SERVICE_START_PENDING)
        {
            service_status_mutex.unlock();
            return;
        }
        break;
    case SERVICE_STOP_PENDING:
        // don't go back to starting or running
        if((dwCurrentState != SERVICE_STOP_PENDING) && (dwCurrentState != SERVICE_STOPPED))
        {
            service_status_mutex.unlock();
            return;
        }
        break;
    case SERVICE_STOPPED:
        // don't call SetServiceStatus anymore after SERVICE_STOPPED has been reported
        service_status_mutex.unlock();
        return;
    }

    switch(dwCurrentState)
    {
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:
        service_status.dwControlsAccepted = 0;
        service_status.dwCheckPoint = service_checkpoint;
        service_checkpoint += 1;
        break;
    case SERVICE_RUNNING:
    case SERVICE_STOPPED:
        service_status.dwCheckPoint = 0;
        service_status.dwControlsAccepted =
            // SERVICE_ACCEPT_POWEREVENT   |
            SERVICE_ACCEPT_STOP         |
            SERVICE_ACCEPT_PRESHUTDOWN;
        break;
    default:
        // shouldn't happen
        service_status_mutex.unlock();
        return;
    }

    last_state = dwCurrentState;
    service_status.dwCurrentState     = dwCurrentState;
    service_status.dwWin32ExitCode    = dwWin32ExitCode;
    service_status.dwWaitHint         = dwWaitHint;

    SetServiceStatus(service_status_handle, &service_status);

    service_status_mutex.unlock();
    return;
}

// called asynchronously by the service manager
static DWORD WINAPI ServiceControlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    DWORD retval = NO_ERROR;
    LOG_TRACE("ServiceControlHandler(%lu, %lu, %p, %p) called, PID %lu Thread %lu",
              dwControl, dwEventType, lpEventData, lpContext, GetCurrentProcessId(), GetCurrentThreadId());
    switch(dwControl)
    {
    // case SERVICE_CONTROL_POWEREVENT: // TODO either this or RegisterSuspendResumeNotification somewhere
    case SERVICE_CONTROL_INTERROGATE:
        // should do nothing but return NO_ERROR
        break;
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_PRESHUTDOWN:
        ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 10000);
        service_stop_requested = true;
        break;
    default:
        retval = ERROR_CALL_NOT_IMPLEMENTED;
        break;
    }
    return retval;
}

// only for errors during service startup
static void LogEvent(const char *fmt, ...)
{
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    int len = vsnprintf(NULL, 0, fmt, args1);
    char *buf = (char*)malloc(len + 1);
    va_end(args1);
    vsnprintf(buf, len + 1, fmt, args2);
    va_end(args2);

    // if we somehow ended up here with a working console then just print it
    if(have_console)
    {
        printf("%s\n", buf);
        free(buf);
        return;
    }

    HANDLE event_source = RegisterEventSource(NULL, service_name);
    if(event_source)
    {
        LPCSTR strings[2] = { service_name, buf };
        ReportEvent(
            event_source,
            EVENTLOG_ERROR_TYPE,
            0,
            1, // error #
            NULL,
            2,
            0,
            strings,
            NULL);
        DeregisterEventSource(event_source);
    }
    else
    {
        // No console, no logmanager, no eventlog, no more ideas. Sometimes it's OK to give up.
    }
    free(buf);
}

static void LogError(const char *fn, DWORD lasterror)
{
    std::string message = std::system_category().message(lasterror);
    LogEvent("%s failed with error code %lu \"%s\"", fn, lasterror, message.c_str());
}

static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    service_status.dwServiceSpecificExitCode = 0;

    started_as_service = true;
    service_status_handle = RegisterServiceCtrlHandlerEx(service_name, ServiceControlHandler, NULL);
    if(service_status_handle == 0)
    {
        LogError("RegisterServiceCtrlHandlerEx", GetLastError());
        return;
    }
    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 30000);

    /*---------------------------------------------------------------------*\
    | Code that needs to be run at startup when we are started as a service |
    | and not as an app goes here.                                          |
    \*---------------------------------------------------------------------*/

    int exitval = common_main(dwArgc, lpszArgv);
    if(exitval != EXIT_SUCCESS)
    {
        LogEvent("OpenRGB finishing with exit code %d", exitval);
    }

    ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

/*--------------------------------------------------------------------------------*\
|                                                                                  |
| main                                                                             |
|                                                                                  |
|     Figures out whether we are started as a service, calls common_main           |
|     one way or another.                                                          |
|                                                                                  |
\*--------------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    started_as_service = false;
    // this will call ServiceMain() if we are started as a service
    bool sscd = StartServiceCtrlDispatcher(service_dispatch_table);
    // ... returns a specific error code otherwise
    DWORD lasterror = GetLastError();
    if(sscd)
    {
        if(started_as_service)
        {
            /*--------------------------------------------------------------------*\
            |  StartServiceCtrlDispatcher reported success, ServiceMain was        |
            |  reached. Don't log anything, it's the normal exit path when a       |
            |  service is stopped.                                                 |
            \*--------------------------------------------------------------------*/
            return EXIT_SUCCESS;
        }
        else
        {
            // should not happen
            LogEvent("StartServiceCtrlDispatcher returned true, but ServiceMain has not been reached.");
            return EXIT_FAILURE;
        }
    }
    // StartServiceCtrlDispatcher reported failure.
    // first, check that the error isn't the one returned when it's called from an app
    if(lasterror != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
    {
        LogError("StartServiceCtrlDispatcher", lasterror);
        return EXIT_FAILURE;
    }
    // now we are quite sure we are not started as a service
    if(started_as_service)
    {
        // ... so this should not happen
        LogEvent("StartServiceCtrlDispatcher returned false, but ServiceMain has been reached.");
        return EXIT_FAILURE;
    }

    /*--------------------------------------------------------------------*\
    | Code that needs to be run at startup when we are started as an app   |
    | and not as a service goes here.                                      |
    \*--------------------------------------------------------------------*/

    /*---------------------------------------------------------*\
    | Attach console output                                     |
    \*---------------------------------------------------------*/
    if(AttachConsole(ATTACH_PARENT_PROCESS))
    {
        /*---------------------------------------------------------*\
        | We are running under some terminal context; otherwise     |
        | leave the GUI and CRT alone                               |
        \*---------------------------------------------------------*/
        freopen("CONIN$",  "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        have_console = true;
    }
    return common_main(argc, argv);
}

/******************************************************************************************\
*                                                                                          *
*   InitializeTimerResolution (Win32)                                                      *
*                                                                                          *
*       On Windows, the default timer resolution is 15.6ms.  For higher accuracy delays,   *
*       the timer resolution should be set to a shorter interval.  The shortest interval   *
*       that can be set is 0.5ms.                                                          *
*                                                                                          *
\******************************************************************************************/

void InitializeTimerResolution()
{
    HMODULE ntdll = LoadLibrary("ntdll.dll");
    if(ntdll != NULL)
    {
        typedef LONG (*NTSETTIMERRESOLUTION)(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
        ULONG CurrentResolution;
        NTSETTIMERRESOLUTION NtSetTimerResolution = (NTSETTIMERRESOLUTION)GetProcAddress(ntdll, "NtSetTimerResolution");
        if(NtSetTimerResolution != NULL)
        {
            NtSetTimerResolution(1000, TRUE, &CurrentResolution);
        }
    }

    // PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION = 4, isn't defined in Win10 headers
    PROCESS_POWER_THROTTLING_STATE PowerThrottlingState { PROCESS_POWER_THROTTLING_CURRENT_VERSION, 4, 0 };
    // take care of the slowdown on Windows 11
    SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &PowerThrottlingState, sizeof(PowerThrottlingState));
}

static void WaitWhileServerOnline(NetworkServer* srv)
{
    while(srv->GetOnline())
    {
        std::this_thread::sleep_for(1s);
        if(service_stop_requested)
        {
            srv->StopServer();
        }
    };
}

static int common_main(int argc, char* argv[])
{
    unsigned int ret_flags;
    if(started_as_service)
    {
        /*------------------------------------------------------------------------*\
        | passing command line arguments to a service is difficult,                |
        | can cause all kinds of trouble and doesn't have a way to warn about them |
        \*------------------------------------------------------------------------*/

        // does autoconnect make sense for a service?
        ret_flags = RET_FLAG_START_SERVER | RET_FLAG_NO_AUTO_CONNECT;
    }
    else
    {
        ret_flags = cli_pre_detection(argc, argv);
    }
    InitializeTimerResolution();
    ResourceManager::get()->Initialize(
        !(ret_flags & RET_FLAG_NO_AUTO_CONNECT),
        !(ret_flags & RET_FLAG_NO_DETECT),
        ret_flags & RET_FLAG_START_SERVER,
        ret_flags & RET_FLAG_CLI_POST_DETECTION);
    if(started_as_service)
    {
        ResourceManager::get()->RegisterDetectionProgressCallback(ServiceStartupProgress, NULL);
    }

    int exitval = startup(argc, argv, ret_flags);

    if(started_as_service)
    {
        ResourceManager::get()->UnregisterDetectionProgressCallback(ServiceStartupProgress, NULL);
    }
    if((ret_flags & RET_FLAG_START_SERVER) && !(ret_flags & RET_FLAG_START_GUI))
    {
        NetworkServer* server = ResourceManager::get()->GetServer();
        if(server && server->GetOnline())
        {
            if(started_as_service)
            {
                ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);
            }
            WaitWhileServerOnline(server);
        }
    }
    ResourceManager::get()->Cleanup();
    LOG_TRACE("OpenRGB finishing with exit code %d", exitval);
    return exitval;
}
