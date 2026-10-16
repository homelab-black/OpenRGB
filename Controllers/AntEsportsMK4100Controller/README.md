# Ant Esports MK4100 support for OpenRGB 1.0

This controller targets USB VID:PID `36AE:FE9C` and the vendor HID interface
with usage page `0xFF00`, usage `0x02`.

## RGB functionality

- Exposes the 104 physical keys from the verified JYK200 104M layout.
- Uses the shared `mk4100_protocol` C core and OpenRGB's HIDAPI transport.
- Sends live custom RGB frames as the original vendor-format seven 56-byte
  chunks, waiting for and matching each keyboard-protocol acknowledgement.
- Uses the vendor single-key `0x14` command for OpenRGB's single-LED update.
- Exposes firmware modes 0 through 16, using the mode names and parameters
  found in the supplied vendor WebHID profile.
- Does not use the experimental no-ACK or pipelined paths.

The mixed-color live bulk write and single-key protocol have been validated on
physical hardware by the project owner. The OpenRGB adapter itself still needs
end-to-end testing on the target machine.

## Build integration

The controller `.cpp` and `.h` files are included by OpenRGB 1.0's recursive
controller source discovery. `OpenRGB.pro` explicitly adds the two `.c` files
because that discovery only includes C++ files.

## Validation and limitations

- The protocol and key-layout core pass the project's mock-device tests and
  AddressSanitizer/UndefinedBehaviorSanitizer test.
- The C++ controller, detector and RGB-controller sources pass standalone
  syntax checks against the OpenRGB 1.0 headers.
- A full qmake/OpenRGB build and GUI detection have not been run in the build
  environment; compile and test them on the target system before relying on it.
- Native firmware-effect selection uses the vendor `0x16` mode-template query
  followed by a `0x0B` configuration write. Validate each effect/parameter on
  the physical keyboard. Direction controls are exposed only for the profile
  modes that declare direction metadata.
- OpenRGB's generic mode-color selector does not have a dedicated
  monochrome-vs-multicolor toggle. For modes that support the vendor's
  `color` flag, Mode-Specific Color maps to monochrome and OpenRGB's alternate
  color mode maps to the vendor's full-color effect.
- Macro storage, key remapping, profiles/layers, factory reset, and firmware
  updating are intentionally outside this RGB controller. They belong in the
  broader native Linux management library/tool, not in OpenRGB's RGB driver.

OpenRGB's device destructor intentionally leaves the last lighting mode/frame
selected by the user active; it does not restore the pre-detection snapshot.
