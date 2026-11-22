# Radio protocol

Defined in [`common/protocol.h`](../common/protocol.h), which both firmwares include.

Packets carry a short header followed by a list of typed readings. A node
sends only the readings it has: a temperature/humidity node sends two, a
weather station can add pressure, wind and rain. The receiver announces each
reading to Home Assistant the first time a node sends it.

The format is shared with the
[AtTinyEnvSensor](https://github.com/jointine-john/AtTinyEnvSensor) project
(ATtiny85 temperature/humidity nodes), so one receiver can listen to both.

## Physical layer

- 433.92 MHz OOK (on-off keying)
- Manchester encoding, 1000 bit/s (500 µs half-bits). The first half-bit
  carries the bit value, the second half its inverse.
- MSB first

## Frame

| Field | Bytes | Value |
|---|---|---|
| Preamble | 4 | `0xAA` × 4, lets the receiver's gain settle |
| Sync | 1 | `0xD4` |
| Length | 1 | Number of body bytes, 4–48 |
| Body | 4–48 | Header, then readings. See below |
| CRC | 1 | CRC-8, polynomial 0x31, init 0xFF, over the length byte and body |
| Trailer | 1 bit | `0`, so the last CRC half-bit ends on an edge |

The original fixed-length format used sync byte `0x2D`. The new sync byte
means a receiver ignores nodes still running the old firmware rather than
misreading them. **Reflash every node and the receiver together.**

## Body header (big-endian)

| Offset | Type | Field |
|---|---|---|
| 0 | uint8 | Node ID |
| 1 | uint8 | Sequence number (wraps) |
| 2–3 | uint16 | Battery voltage, mV (0 = not battery powered) |

## Readings

Each reading is a type byte followed by its value (big-endian). The top two
bits of the type give the value's size, so a receiver can skip types it
doesn't know yet:

| Bits 7–6 | Value size |
|---|---|
| `00` | 1 byte |
| `01` | 2 bytes |
| `10` | 4 bytes |
| `11` | Reserved: the receiver rejects the packet |

| Type | Name | Value | Home Assistant |
|---|---|---|---|
| `0x01` | Status | uint8 flags. Bit 0: T/H sensor error. Sent only when something is wrong | Logged |
| `0x41` | Temperature | int16, centi-°C | °C |
| `0x42` | Humidity | uint16, centi-% RH | % |
| `0x43` | Pressure | uint16, deci-hPa (station pressure, not corrected to sea level) | hPa |
| `0x44` | Wind speed | uint16, centi-m/s, average over the reporting interval | m/s |
| `0x45` | Wind gust | uint16, centi-m/s, highest short-window average in the interval | m/s |
| `0x46` | Wind direction | uint16, deci-degrees from north (0–3599) | ° |
| `0x87` | Rain | uint16 tip count since the node started (wraps), then uint16 rainfall per tip in µm | mm, total increasing |

Rain is sent as a running tip count rather than "rain since last packet", so a
lost packet loses no rain: the next one carries the total. The node also sends
its bucket size (e.g. 279 µm for a common 0.2794 mm tipper), so the receiver
needs no per-node settings. Home Assistant's `total_increasing` sensor
handles the count restarting when a node is reset.

New reading types take the next free ID in the low six bits, with the size
bits set to match the value.

### Example: temperature/humidity node

17 bytes on air, 137 ms per frame:

```
AA AA AA AA   D4    0A    01    2C    0B B8      41 08 6A    42 11 A8    29
preamble      sync  len   node  seq   battery    temp        humidity    CRC
                    10    1     44    3000 mV    21.54 °C    45.20 %
```

On a sensor error the node sends `01 01` (status: sensor error) instead of
the temperature and humidity.

## Reliability

There is no acknowledgement. Instead:

- Each frame is sent 3 times, 20 ms apart. The receiver de-duplicates on
  (node, seq) within 5 seconds.
- Nodes randomly add 8 s to some intervals so sensors that start together
  drift apart instead of colliding every time.
- The decoder runs two alignments in parallel and resynchronises on any pulse
  that isn't one or two half-bits long, so noise between packets is ignored.
  An impossible length byte also resets it straight away.
- A packet whose CRC is valid but whose body is malformed (a reading running
  past the end, or the reserved size code) is dropped and logged.

## Regulatory

433.92 MHz at well under 1% duty cycle is within UK/EU SRD (IR2030) limits.
