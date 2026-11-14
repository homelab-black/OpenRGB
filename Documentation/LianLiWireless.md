# Lian Li wireless protocol notes

This controller supports one Lian Li wireless transmitter (`0416:8040`) and
one receiver (`0416:8041`) connected through WinUSB/libusb. It currently
exposes bound UNI FAN SL V3 Wireless LED groups in Direct and Static modes.
The driver only sends lighting uploads and clock keep-alives; it does not send
fan-speed, pairing, firmware, or motherboard-sync commands.

## Discovery and ownership

The transmitter and receiver use bulk endpoint `0x01` for writes and `0x81`
for reads. A discovery response starts with USB command `0x10`, followed by a
record count and 42-byte device records. The relevant record fields are:

- device MAC, master MAC, RF channel, and receiver slot;
- current effect ID and command sequence;
- fan count, fan type bytes, RPM, PWM, and motherboard RGB-sync state.

Only records with a valid unicast device MAC and addressed receiver slot are
accepted. The detector also requires the record's master MAC to match the
attached transmitter. It refuses groups in motherboard RGB-sync mode and fan
types that have not been validated.

## Lighting uploads

An RGB profile is compressed with tinyuz and split into 64-byte RF packets.
Each RF packet is then divided into four 64-byte USB transfers carrying the
RF channel and receiver slot. The header packet is repeated once, matching
the observed vendor sequence. A deterministic effect ID covers the compressed
payload, dimensions, timing, and a variant salt. The runtime resends until a
fresh discovery record reports that effect ID.

One worker serializes all access to the shared dongle pair. It prioritizes
clock keep-alives, refreshes channel and receiver addressing from discovery,
debounces master changes, stops writes after sustained absence, and revalidates
the transmitter identity after a USB reconnect. Rescanning stops the worker,
rebuilds its fan-group runtimes from fresh discovery data, and restarts it.

## Sources and validation

The packet layout was established from local USB captures and compared with
the MIT-licensed [`sgtaziz/lian-li-linux`](https://github.com/sgtaziz/lian-li-linux)
implementation, most recently at revision
`178a494c6f195d22eefd6dd73455b5fe4a352df7`.

The offline suite in `tests/lianli_wireless` covers packet construction,
malformed discovery data, compression round trips, ownership changes,
disconnect recovery, resend limits, group replacement, and cancellation.
Hardware validation used one `SLV3TX_V1.6`, one `SLV3RX_V1.6`, and a bound
three-fan UNI FAN SL V3 Wireless group.
