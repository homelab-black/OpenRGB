# Lian Li wireless offline tests

These tests cover packet construction, compression round trips, discovery parsing,
upload acknowledgement/retry handling, transport reconnect recovery, and the service
scheduler using a fake clock/link. They do not communicate with USB hardware.
On Windows, an optional decoder compatibility check uses an already-installed
L-Connect 3 yuz.dll; it is skipped if that DLL is absent.

Use a compiler environment compatible with your qmake installation. From a separate
build directory, run:

```
qmake /path/to/OpenRGB/tests/lianli_wireless/lianli_wireless.pro CONFIG+=release
make
./lianli_protocol_test
```

For MSVC, use `nmake` instead of `make`, then run `lianli_protocol_test.exe`.
Qt is needed only for qmake; the test executable does not link Qt.
A nonzero exit code indicates a failure. The number of checks depends on whether
the optional vendor decoder is present. Passing this suite does not validate USB
access, device detection, live RGB output, or the RGBController adapter.
