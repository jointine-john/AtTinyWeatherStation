# Building and flashing

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).

## Sensor node

The ATtiny is programmed over ISP with a USBasp or an Arduino running
ArduinoISP. See [Programming the ATtiny](programming.md) for wiring.

Choose the PlatformIO environment that matches your chip and sensor board (see
[Sensors](sensors.md) for the differences between the sensors):

| Environment | Chip | Sensor | Example boards |
|---|---|---|---|
| `htu21d` (default) | ATtiny85 | HTU21D, SHT21 or Si7021 | GY-21, GY-213V |
| `sht40` | ATtiny85 | Sensirion SHT40 | Adafruit 4885, DFRobot Fermion |
| `attiny84_htu21d` | ATtiny84/84A | HTU21D, SHT21 or Si7021 | GY-21, GY-213V |
| `attiny84_sht40` | ATtiny84/84A | Sensirion SHT40 | Adafruit 4885, DFRobot Fermion |

Pin-outs for each chip are in `sensor-node/src/board.h` and
[Wiring](wiring.md). The ATtiny84 builds currently send temperature and
humidity only; wind and rain come next (see [Future plans](future-plans.md)).

```sh
cd sensor-node
pio run -t fuses                                   # once per new chip
PLATFORMIO_BUILD_FLAGS="-DNODE_ID=1" pio run -t upload            # HTU21D/GY-21
PLATFORMIO_BUILD_FLAGS="-DNODE_ID=2" pio run -e sht40 -t upload   # SHT40
PLATFORMIO_BUILD_FLAGS="-DNODE_ID=3" pio run -e attiny84_sht40 -t upload   # ATtiny84
```

Give each sensor a unique `NODE_ID` (1–255). If the build doesn't match the
board, the receiver logs `SENSOR ERROR` for that node but still reports its
battery voltage.

### Build options

| Flag | Default | Meaning |
|---|---|---|
| `NODE_ID` | 1 | Sensor ID reported to the receiver |
| `INTERVAL_S` | 300 | Seconds between readings (rounded to 8 s steps) |
| `BANDGAP_MV` | 1100 | ATtiny internal reference, for battery calibration |

### Calibrating the battery reading

The ATtiny's internal 1.1 V reference is only ±10%. Measure VCC with a meter,
compare with the value the receiver reports, and rebuild with
`-DBANDGAP_MV=<1100 × meter / reported>`.

## Receiver

```sh
cd receiver
cp src/secrets.example.h src/secrets.h   # fill in WiFi + MQTT
pio run -t upload && pio device monitor
```

`secrets.h` is git-ignored.

### Home Assistant

Each sensor appears in Home Assistant through MQTT discovery as a device
"ATtiny sensor N". Its entities are created as the node first sends each
reading: Temperature, Humidity and Battery (V) for a T/H node, plus Pressure,
Wind speed, Wind gust, Wind direction and Rain for nodes that send them (see
[Radio protocol](protocol.md#readings)). They become unavailable after 20
minutes without data. Raw JSON is published on `attiny_th/<id>/state`, with
only the readings that node sent:

```json
{"seq":12,"temperature":21.53,"humidity":45.20,"battery":2.981}
```

The serial monitor logs every decoded packet, which is the quickest way to
check the radio link.

### Loopback test (no sensor needed)

The `loopback` environment adds a test transmitter to the receiver firmware. It
sends a fake sensor packet (node 250) every 30 seconds on **GPIO5**, using
exactly the ATtiny85's encoding. The packet carries every reading type
(temperature, humidity, pressure, wind and rain), so it tests all of them.

1. Fit a jumper wire from **GPIO5 to GPIO4**. Disconnect any receiver module's
   DATA pin from GPIO4 first.
2. Flash and watch the serial monitor:
   ```sh
   cd receiver
   pio run -e loopback -t upload && pio device monitor
   ```
3. Each `loopback: sent` line should be followed by a matching decoded line:
   ```
   loopback: sent node 250 seq 1 (29 bytes): 20.10 C, rain tips 1
   node 250: {"seq":1,"temperature":20.10,"humidity":50.00,"pressure":1013.2,"wind_speed":3.50,"wind_gust":7.20,"wind_direction":22.5,"rain":0.28,"battery":3.000}
   ```
4. "ATtiny sensor 250" appears in Home Assistant with all eight entities. The
   temperature steps from 20.00 to 21.90 °C, the wind direction turns 22.5°
   per packet and the rain total creeps up.

To test over the air, wire GPIO5 to an STX882's DATA pin (VCC to 3V3, GND to
GND) and connect the receiver module to GPIO4 as normal.

Flash the normal build (`pio run -t upload`) when you're done. Delete the
node 250 device in Home Assistant if you don't want to keep it.

## Decoder test (PC)

Needs `g++` and `make`.

```sh
make -C test
```

It feeds the receiver's decoder 20,000 simulated packets, each with a random
mix of readings, adding clock error, pulse distortion, jitter and noise. It
checks that clean packets decode and parse back to the readings sent, that
corrupted ones are rejected, and that malformed packet bodies are dropped.
