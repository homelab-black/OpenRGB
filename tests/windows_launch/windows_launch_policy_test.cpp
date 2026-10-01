#include "../../startup/WindowsLaunchPolicy.h"
#include "../../startup/WindowsLaunchLock.h"
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

static void Check(bool condition, const char* name, int& failures)
{
    if(!condition)
    {
        std::printf("FAIL: %s\n", name);
        ++failures;
    }
}

int main()
{
    using A = WindowsLaunchAction;
    struct Case { const char* name; bool others, server, gui, request_gui; A expected; };
    const Case cases[] = {
        {"nothing running", false, false, false, true, A::Start},
        {"server only", true, true, false, true, A::ConnectGui},
        {"GUI and server", true, true, true, true, A::Restart},
        {"GUI only", true, false, true, true, A::Restart},
        {"server still starting", true, false, false, true, A::Restart},
        {"duplicate headless server", true, true, false, false, A::Restart},
        {"unidentified port owner", false, true, false, true, A::Restart},
    };
    int failures = 0;
    for(const auto& c : cases)
        if(GetWindowsLaunchAction(c.others, c.server, c.gui, c.request_gui) != c.expected)
        {
            std::printf("FAIL: %s\n", c.name);
            ++failures;
        }

    const std::vector<std::string> remote_args = {
        "OpenRGB.exe", "--gui", "--client", "192.168.1.20:7000", "--nodetect"
    };
    WindowsLaunchIntent remote = AnalyzeWindowsLaunchArguments(remote_args);
    Check(remote.gui_requested, "remote client requests GUI", failures);
    Check(remote.explicit_client, "remote client is detected", failures);
    Check(!remote.guard_required, "nodetect remote client bypasses local instance guard", failures);

    std::vector<std::string> preserved = BuildWindowsLaunchArguments(remote_args,
        WindowsLaunchAction::ConnectGui);
    Check(preserved == remote_args, "remote client arguments are preserved", failures);

    const std::vector<std::string> attach_args = { "OpenRGB.exe", "--gui" };
    const std::vector<std::string> expected_attach = {
        "OpenRGB.exe", "--gui", "--nodetect", "--client", "127.0.0.1:6742"
    };
    Check(BuildWindowsLaunchArguments(attach_args, WindowsLaunchAction::ConnectGui)
            == expected_attach,
          "second GUI becomes local SDK client", failures);

    const std::vector<std::string> saved_client_args = { "OpenRGB.exe" };
    const std::vector<std::string> expected_first_gui = {
        "OpenRGB.exe", "--gui", "--server"
    };
    Check(BuildWindowsLaunchArguments(saved_client_args, WindowsLaunchAction::Start)
            == expected_first_gui,
          "first GUI keeps configured autoconnections enabled", failures);

    const std::vector<std::string> custom_port_args = {
        "OpenRGB.exe", "--gui", "--server-port", "7000"
    };
    Check(AnalyzeWindowsLaunchArguments(custom_port_args).server_requested,
          "custom SDK port is a server request", failures);
    Check(BuildWindowsLaunchArguments(custom_port_args, WindowsLaunchAction::Start)
            == custom_port_args,
          "custom SDK port is preserved", failures);
    Check(GetWindowsLaunchAction(true, true, false, true, true)
            == WindowsLaunchAction::Restart,
          "explicit server request never becomes an SDK client", failures);

    const std::vector<std::string> help_args = { "OpenRGB.exe", "--help" };
    const std::vector<std::string> version_args = { "OpenRGB.exe", "--version" };
    Check(!AnalyzeWindowsLaunchArguments(help_args).guard_required,
          "help bypasses local instance guard", failures);
    Check(!AnalyzeWindowsLaunchArguments(version_args).guard_required,
          "version bypasses local instance guard", failures);

    Check(CanStartWindowsService(false, false), "service starts when machine is clear", failures);
    Check(!CanStartWindowsService(true, false), "service refuses another OpenRGB process", failures);
    Check(!CanStartWindowsService(false, true), "service refuses occupied SDK port", failures);
    char lock_name[128];
    std::snprintf(lock_name, sizeof(lock_name), "Local\\OpenRGB.LaunchTest.%lu", GetCurrentProcessId());
    WindowsLaunchLock first;
    if(first.Acquire(lock_name) != WAIT_OBJECT_0)
        ++failures;
    DWORD second_result = WAIT_FAILED;
    std::thread contender([&] {
        WindowsLaunchLock second;
        second_result = second.Acquire(lock_name);
    });
    contender.join();
    if(second_result != WAIT_TIMEOUT)
    {
        std::puts("FAIL: simultaneous startup admitted");
        ++failures;
    }
    first.Release();
    std::thread next_launch([&] {
        WindowsLaunchLock next;
        second_result = next.Acquire(lock_name);
    });
    next_launch.join();
    if(second_result != WAIT_OBJECT_0)
    {
        std::puts("FAIL: startup lock was not released");
        ++failures;
    }
    std::printf("launch policy, argument preservation, service guard, and lock checks: %d failures\n", failures);
    return failures ? 1 : 0;
}
