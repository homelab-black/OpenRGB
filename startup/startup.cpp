/*---------------------------------------------------------*\
| startup.cpp                                               |
|                                                           |
|   Startup for the OpenRGB application                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "cli.h"
#include "ResourceManager.h"
#include "NetworkServer.h"
#include "startup.h"

#include <QApplication>

#include "OpenRGBDialog.h"

#ifdef __APPLE__
#include "macutils.h"
#endif

#ifdef __linux__
#include <csignal>
#include <sys/socket.h>
#include <unistd.h>
#include <QSocketNotifier>
#endif

/******************************************************************************************\
*                                                                                          *
*   Linux signal handler                                                                   *
*                                                                                          *
*       Only async-signal-safe calls are allowed here, so the signal is passed to the      *
*       event loop through a socket pair and the application quits from there              *
*                                                                                          *
\******************************************************************************************/
#ifdef __linux__
static int signal_sockets[2];

void sigHandler(int s)
{
    char signal_number = (char)s;

    std::signal(s, SIG_DFL);

    if(write(signal_sockets[1], &signal_number, sizeof(signal_number)) != sizeof(signal_number))
    {
        _exit(EXIT_FAILURE);
    }
}
#endif

/******************************************************************************************\
*                                                                                          *
*   startup                                                                                *
*                                                                                          *
*       Opens the main windows or starts the server                                        *
*                                                                                          *
\******************************************************************************************/
int startup(int argc, char* argv[], unsigned int ret_flags)
{
    /*-----------------------------------------------------*\
    | Initialize exit value, which will be returned on exit |
    | in main()                                             |
    \*-----------------------------------------------------*/
    int exitval = EXIT_SUCCESS;

    /*-----------------------------------------------------*\
    | Before opening GUI, wait for automatic connection so  |
    | that settings and profiles can be updated from server |
    \*-----------------------------------------------------*/
    ResourceManager::get()->WaitForInitialization();

    /*-----------------------------------------------------*\
    | If the command line parser indicates that the GUI     |
    | should run, or if there were no command line          |
    | arguments, start the GUI.                             |
    \*-----------------------------------------------------*/
    if(ret_flags & RET_FLAG_START_GUI)
    {
        LOG_TRACE("[main] initializing GUI");

        /*-------------------------------------------------*\
        | Enable high DPI scaling support (Qt5 only)        |
        \*-------------------------------------------------*/
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps,    true);
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
        #endif

        /*-------------------------------------------------*\
        | Enable high DPI fractional scaling support on     |
        | Windows                                           |
        \*-------------------------------------------------*/
        #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0) && defined(Q_OS_WIN)
            QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
        #endif

        /*-------------------------------------------------*\
        | Create Qt application                             |
        \*-------------------------------------------------*/
        QApplication a(argc, argv);
        QGuiApplication::setDesktopFileName("org.openrgb.OpenRGB");
        LOG_TRACE("[startup] QApplication created");

        /*-------------------------------------------------*\
        | Main UI widget                                    |
        \*-------------------------------------------------*/
        OpenRGBDialog dlg;
        LOG_TRACE("[startup] Dialog created");

        if(ret_flags & RET_FLAG_I2C_TOOLS)
        {
            dlg.AddI2CToolsPage();
        }

        dlg.AddClientTab();

        if(ret_flags & RET_FLAG_START_MINIMIZED)
        {
#ifdef _WIN32
            /*---------------------------------------------*\
            | Show the window always, even if it will       |
            | immediately be hidden.  On Windows, events    |
            | are not delivered to nativeEventFilter (for   |
            | SuspendResume) until the window has been      |
            | shown once.                                   |
            |                                               |
            | TODO Try using                                |
            | RegisterSuspendResumeNotification instead,    |
            | that should work in headless mode too         |
            \*---------------------------------------------*/
            dlg.showMinimized();
#endif
#ifdef __APPLE__
            MacUtils::ToggleApplicationDocklessState(false);
#endif
            dlg.hide();
        }
        else
        {
            dlg.show();
        }

        LOG_TRACE("[main] Ready to exec() the dialog");

#ifdef __linux__
        if(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, signal_sockets) == 0)
        {
            QSocketNotifier* signal_notifier = new QSocketNotifier(signal_sockets[0], QSocketNotifier::Read, &a);

            QObject::connect(signal_notifier, &QSocketNotifier::activated, [signal_notifier]()
            {
                char signal_number;

                signal_notifier->setEnabled(false);

                if(read(signal_sockets[0], &signal_number, sizeof(signal_number)) == sizeof(signal_number))
                {
                    LOG_INFO("[startup] Received signal %d, exiting", signal_number);
                }

                qApp->quit();
            });

            std::signal(SIGINT,  sigHandler);
            std::signal(SIGTERM, sigHandler);
        }
        else
        {
            LOG_WARNING("[startup] Could not create signal socket pair, SIGINT and SIGTERM will exit without cleanup");
        }
#endif

        exitval = a.exec();
    }
    else
    {
        /*-------------------------------------------------*\
        | If no GUI is needed, we let the background        |
        | threads run as long as they need, but we need to  |
        | AT LEAST wait for initialization to finish        |
        \*-------------------------------------------------*/
        ResourceManager::get()->WaitForInitialization();

        if(ret_flags & RET_FLAG_START_SERVER)
        {
            NetworkServer* server = ResourceManager::get()->GetServer();
            if(server)
            {
                exitval = !server->GetOnline();
            }
            else
            {
                exitval = EXIT_FAILURE;
            }
        }
    }

    return(exitval);
}
