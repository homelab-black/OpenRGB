# AMD Radeon RX 7900 XTX reference RGB

Linux support for the reference board's Cooler Master RGB light bar:

- PCI device `1002:744C`, subsystem `1002:0E3B`
- Adapter `AMDGPU SMU 1` belonging to that GPU
- Seven-bit I²C address `0x5A` (`0xB4`/`0xB5` in the vendor application)

Only this board and adapter are probed. SMU 0, display/DDC/AUX/OEM buses,
other board IDs, and the ordinary Windows ADL I²C backend are not supported.
The Linux adapter number is discovered rather than hardcoded.

## Supported controls

One logical **Light Bar** LED controls all 15 physical LEDs together:
Static RGB, Breathing, Spectrum Cycle, and Off. Static and Breathing support
brightness 0–255; Spectrum Cycle supports 0–127. Effect speeds use captured
defaults. Direct/per-pixel control, adjustable speed, Morse code, and saving
to device flash are not implemented.

Detection sends only the non-lighting identification query. Initialization
and lighting commands require successful identification and an explicit
update. An I/O, framing, or command-echo error stops the operation; an update
failure blocks further writes until detection succeeds again. Transfers and
multi-command updates are serialized per controller, without blind retries.

## Protocol sources

- [Device request #3037 and application captures](https://gitlab.com/CalcProgrammer1/OpenRGB/-/issues/3037#note_1710647123)
- Disassembly of [AMD's official RGB utility](https://drivers.amd.com/drivers/installer/comp/sb/amd_rx_7900_xtx_rgb_led_20241008.exe)
- Identification, command acknowledgements, and configuration readback on hardware

The vendor application uses `ADL_Display_WriteAndReadSMUI2C`, ADL line 0,
100 kHz, and offset 0. ADL line numbering must not be assumed to match Linux
SMU numbering: Linux SMU 1 is the working adapter on the tested card.

## Framing

Each command contains 64 payload bytes, sent as three independent raw I²C
writes without SMBus byte-count framing or repeated starts:

| Packet | Prefix | Payload offsets | Total length |
| --- | --- | --- | --- |
| 1 | `40` | 0–22 | 24 |
| 2 | `57` | 23–45 | 24 |
| 3 | `AE` | 46–63 | 19 |

Every read starts with `0xFF - payload_offset`. The vendor application reads
24, 24, and 19 bytes, prefixed `FF`, `E8`, and `D1`. These long reads failed
with `EIO` on the tested Linux SMU adapter, whose kernel driver fixes the
speed at 400 kHz. The underlying cause has not been established.

The implementation instead uses 64 separate **two-byte reads**: one prefix
and one payload byte each. Prefixes must decrease from `FF` through `C0`.
The first four reconstructed reply bytes must echo the request. Writes are
paced with 10 ms gaps, reads with 1 ms gaps. This transport works without
changing the kernel or GPU firmware.

## Commands

- Identification: `12 02`, zero-padded; reply byte 4 must be `01`.
- Initial LED-controller setup: `41 80`, `51 96`, then apply.
- Effect: `51 2C 01 00`; byte 4 channel, byte 5 speed, byte 6 flags,
  byte 7 effect parameter, byte 9 brightness, bytes 10–12 RGB. Other bytes
  are `FF`, except bytes 13–15 which are zero.
- Mapping: `51 A0 01 00 00 03 00 00 05 06`, followed by 15 channel IDs
  and zero padding.
- Apply: `51 28 00 00 E0`, zero-padded.
- Read effect: `52 2C 01 00`, byte 4 channel, remaining bytes `FF`.
- Read mapping: `52 A0 01 00 00 03`, zero-padded.

| Mode | Channel | Speed | Flags | Byte 7 | Maximum brightness |
| --- | --- | --- | --- | --- | --- |
| Static | `00` | `FF` | `00` | `FF` | `FF` |
| Breathing | `01` | `31` | `20` | `03` | `FF` |
| Spectrum Cycle | `02` | `80` | `00` | `FF` | `7F` |
| Off | `FE` | — | — | — | — |

Off changes only the mapping. Spectrum Cycle uses white RGB values.

## Validation

Tested on Fedora Silverblue 44, kernel `7.2.11-200.fc44.x86_64`, SMU firmware
`78.135.0`. Static red was visually confirmed. Independent hardware queries
after OpenRGB CLI updates confirmed blue/100%, green/20%, breathing red/60%,
spectrum cycle/50%, off, and white/100%, including all 15 channel mappings.
The other effects were not separately visually confirmed. No GPU reset,
driver unload, firmware update, kernel modification, or reboot was needed.
Windows support has not been implemented or tested.

Hardware-free regression tests:

```sh
sh tests/run-rx7900xtx-tests.sh
```

The tests exercise the real controller with a mock adapter: exact PCI/bus
gating, write framing, all 64 read prefixes, captured payloads, modes,
brightness, malformed/short replies, and fail-closed behavior. They do not
access hardware. They can also be compiled with AddressSanitizer and
UndefinedBehaviorSanitizer.
