# Wiring

## Sensor node (ATtiny85 proof of concept)

![Sensor node schematic](images/sensor-schematic.svg)

U2 can be an SHT40 or a GY-21 (HTU21D/SHT21) board: both use the pins VIN, GND,
SCL and SDA. Build the matching firmware (see [Sensors](sensors.md)).

VCC and GND symbols all join the same battery rails. Blue labels are net names:
pins with the same label are connected (J1 is wired to U1 this way).

Pin-level summary:

```
                 ATtiny85
              +-----\/-----+
   (ISP RST) -| 1 PB5  VCC 8|- Battery +  --- 100nF --- GND
     unused  -| 2 PB3  PB2 7|- sensor SCL
     unused  -| 3 PB4  PB1 6|- STX882 DATA
        GND  -| 4 GND  PB0 5|- sensor SDA
              +-------------+

sensor VIN -> Battery +      STX882 VCC -> Battery +
sensor GND -> GND            STX882 GND -> GND, ANT -> 17.3 cm wire
```

Fit the 10 µF capacitor across the battery terminals.

No voltage regulator: everything runs straight off the cells (3.2 V fresh → ~2.0 V flat).
The transmitter draws nothing measurable while its DATA pin is low.

PB0/PB1/PB2 double as the ISP MOSI/MISO/SCK pins, so you can reprogram in-circuit
(the transmitter will chirp during upload, which is harmless). See
[Programming the ATtiny](programming.md#reprogramming-a-finished-sensor-in-place).

## Sensor node (ATtiny84)

The same circuit on an ATtiny84 (build `attiny84_sht40` or `attiny84_htu21d`).
Only the pins change; see `sensor-node/src/board.h`. Pins marked "station"
are reserved for the wind and rain hardware and are driven low by the
firmware until that code exists, so leave them unconnected for now.

```
                            ATtiny84
                        +------\/------+
            Battery +  -| 1 VCC  GND 14|-  GND
 anemometer (station)  -| 2 PB0  PA0 13|-  vane input (station)
               unused  -| 3 PB1  PA1 12|-  vane power (station)
            (ISP RST)  -| 4 PB3  PA2 11|-  unused
               unused  -| 5 PB2  PA3 10|-  rain gauge (station)
               unused  -| 6 PA7  PA4  9|-  sensor SCL / ISP SCK
sensor SDA / ISP MOSI  -| 7 PA6  PA5  8|-  STX882 DATA / ISP MISO
                        +--------------+
```

Fit the 100 nF capacitor across pins 1 and 14, as close to the chip as
possible, and the 10 µF across the battery. Sensor and transmitter power
connect as on the ATtiny85.

## Stripboard layout (sensor node)

![Stripboard layout](images/stripboard-layout.svg)

Board size: 8 strips × 24 holes. Columns are numbered 1–24 from the left and
rows A–H from the top, **viewed from the component side**.

The ATtiny85 sits across a cut, so each of its rows carries two different nets:

| Row | Left of cut (cols 1–4) | Right of cut (cols 6–24) |
|---|---|---|
| A | – | – (spare) |
| B | RESET (whole strip) | |
| C | pin 1 RESET | pin 8 **VCC** |
| D | pin 2 (unused) | pin 7 **SCL / SCK** |
| E | pin 3 (unused) | pin 6 **DATA / MISO** |
| F | pin 4 GND | pin 5 **SDA / MOSI** |
| G | **GND** (whole strip) | |
| H | – | – (spare) |

### Track cuts

Cut **column 5 on rows C, D, E and F** (4 cuts, under the chip). Do **not** cut
rows B or G.

When you turn the board over to cut from the copper side, it is mirrored
left-to-right: column 5 is then 5th from the **right**. Check every cut with a
multimeter afterwards. Pin 1 must not connect to pin 8, 2 to 7, 3 to 6, or 4 to 5.

### Wire links

| Link | From | To | Joins |
|---|---|---|---|
| 1 | C2 | B2 | RESET (pin 1) to row B |
| 2 | F3 | G3 | GND (pin 4) to row G |

### Components

| Ref | Part | Holes | Notes |
|---|---|---|---|
| U1 | 8-pin DIP socket | C4–F4, C7–F7 | Notch towards row B, pin 1 at C4 |
| C2 | 100 nF | C9, G9 | Bend the leads to span 4 rows |
| J1 | 6-pin straight header | B11–G11 | RST, VCC, SCK, MISO, MOSI, GND (top to bottom) |
| U2 | wires to sensor breakout | C15 VIN, D15 SCL, F15 SDA, G15 GND | SHT40 or GY-21 (same pin names). Use any wire length; mount the sensor away from the battery |
| STX882 | wires to module | C18 VCC, E18 DATA, G18 GND | Check your module's pin labels |
| C1 | 10 µF | C21, G21 | |
| BT1 | battery leads | C23 red (+), G23 black (−) | Holder switch in the red lead |

J1 is a single-row header, so connect the USBasp or Arduino with jumper wires using
the pin names. See [Programming the ATtiny](programming.md).

### Build order

1. Make the 4 track cuts and check them with a meter.
2. Fit the two wire links.
3. Fit the IC socket, capacitors and J1 header.
4. Fit the battery, sensor and STX882 wires.
5. Before inserting the chip, power up and check for about 3 V between sockets
   C7 (VCC) and F4 (GND).
6. Insert the ATtiny85, pin 1 (dot) at C4.

## Receiver

Either receiver module works with the same firmware.

![Receiver schematic](images/receiver-schematic.svg)

### RXB6

| RXB6 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| DATA (either DATA pin) | GPIO4 |
| ANT | 17.3 cm wire |

### SRX882

The SRX882 often comes in a kit with the STX882 transmitter. Wire it like the
RXB6, plus the **CS** (enable) pin:

| SRX882 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| DATA | GPIO4 |
| **CS** | **3V3** |
| ANT (either one) | 17.3 cm wire or the supplied coil antenna |

- **CS** is an active-high enable. Low puts the receiver to sleep. Some boards
  enable it when left floating, but tie it to 3V3 so it can't drift low and
  silently stop reception.
- Pin order differs between module versions, so wire by the labels printed on
  the board, not by position.
- The module has two ANT pads. Connect the antenna to one only.
- The coil antenna is compact and fine indoors. A straight 17.3 cm wire usually
  gives more range.

### Pin choice

To use a different GPIO, change `RX_PIN` in `receiver/src/main.cpp`. Avoid the
ESP32's strapping pins (0, 2, 12, 15) and the flash pins (6–11). GPIO4, GPIO5,
GPIO18 and GPIO19 are safe choices.

## Antennas

A straight 17.3 cm piece of solid-core wire (a quarter wavelength at 433.92 MHz)
on both transmitter and receiver. Keep it straight and away from the batteries
and ground wiring where possible; range drops sharply with no antenna at all.
