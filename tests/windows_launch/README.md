# Windows launch policy tests

From an MSVC developer command prompt, use a separate build directory:

```
qmake C:\path\to\OpenRGB\tests\windows_launch\windows_launch.pro CONFIG+=release
nmake
windows_launch_policy_test.exe
```

The test checks seven startup-policy combinations and real Windows named-mutex
contention/release using two threads. It does not start or terminate OpenRGB
processes, access devices, or open an SDK port. Exit code 0 means all checks passed.

This suite does not validate service/GUI coordination, argument rewriting, remote
SDK clients, CLI-only commands, or cross-user/elevation behavior.
