#pragma once

#include <string>
#include <vector>

enum class WindowsLaunchAction { Start, ConnectGui, Restart };

inline WindowsLaunchAction GetWindowsLaunchAction(bool other_processes, bool server_running,
                                                   bool gui_running, bool gui_requested,
                                                   bool server_requested = false)
{
    if(other_processes && server_running && !gui_running && gui_requested && !server_requested)
    {
        return WindowsLaunchAction::ConnectGui;
    }
    return (other_processes || server_running) ? WindowsLaunchAction::Restart : WindowsLaunchAction::Start;
}

struct WindowsLaunchIntent
{
    bool gui_requested   = false;
    bool server_requested = false;
    bool explicit_client = false;
    bool no_detect       = false;
    bool information_only = false;
    bool guard_required  = true;
};

inline bool WindowsLaunchHasArgument(const std::vector<std::string>& args,
                                     const char* argument)
{
    for(size_t i = 1; i < args.size(); i++)
    {
        if(args[i] == argument)
        {
            return true;
        }
    }
    return false;
}

inline WindowsLaunchIntent AnalyzeWindowsLaunchArguments(const std::vector<std::string>& args)
{
    WindowsLaunchIntent intent;
    bool config_only = true;

    for(size_t i = 1; i < args.size(); i++)
    {
        const std::string& arg = args[i];

        if(arg == "--gui" || arg == "--startminimized" || arg == "--i2c-tools")
        {
            intent.gui_requested = true;
        }
        if(arg == "--server" || arg == "--server-port" || arg == "--server-host")
        {
            intent.server_requested = true;
        }
        if(arg == "--client")
        {
            intent.explicit_client = true;
        }
        if(arg == "--nodetect")
        {
            intent.no_detect = true;
        }
        if(arg == "--help" || arg == "-h" || arg == "--version" || arg == "-V")
        {
            intent.information_only = true;
        }

        if(arg == "--config" || arg == "--loglevel" || arg == "--client"
        || arg == "--server-port" || arg == "--server-host"
        || arg == "--autostart-enable")
        {
            i++;
            config_only = false;
            continue;
        }

        if(arg != "--localconfig" && arg != "--noautoconnect" && arg != "--nodetect"
        && arg != "--verbose" && arg != "-v" && arg != "--very-verbose" && arg != "-vv"
        && arg != "--yolo" && arg != "--print-source")
        {
            config_only = false;
        }
    }

    intent.gui_requested = intent.gui_requested || config_only;
    intent.guard_required = !intent.information_only
                         && !(intent.explicit_client && intent.no_detect
                              && !intent.server_requested);
    return intent;
}

inline std::vector<std::string> BuildWindowsLaunchArguments(
    const std::vector<std::string>& input, WindowsLaunchAction action)
{
    std::vector<std::string> args = input;
    WindowsLaunchIntent intent = AnalyzeWindowsLaunchArguments(input);

    if(!intent.gui_requested)
    {
        return args;
    }

    if(!WindowsLaunchHasArgument(args, "--gui"))
    {
        args.push_back("--gui");
    }

    if(action == WindowsLaunchAction::ConnectGui)
    {
        if(!intent.no_detect)
        {
            args.push_back("--nodetect");
        }
        if(!intent.explicit_client)
        {
            args.push_back("--client");
            args.push_back("127.0.0.1:6742");
        }
    }
    else if(action == WindowsLaunchAction::Start
         && !intent.server_requested && !intent.explicit_client)
    {
        args.push_back("--server");
    }

    return args;
}

inline bool CanStartWindowsService(bool other_processes, bool server_running)
{
    return !other_processes && !server_running;
}
