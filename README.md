# ATtiny Weather Station

A battery-powered weather station reporting over 433 MHz to an ESP32 receiver,
which publishes to MQTT with Home Assistant auto-discovery. The goal is an
ATtiny84 station recording wind speed, gust and direction, rainfall, temperature
and humidity, with the receiver adding pressure and a display. See
[Future plans](docs/future-plans.md).

**Status:** the radio protocol and receiver already handle wind and rain. The
node firmware is still the ATtiny85 temperature/humidity proof of concept
(ATtiny85 + SHT40 or HTU21D + STX882 on 2×AA, ~140–150 mAh a year), and is the
starting point for the ATtiny84 port. The docs below describe that proof of
concept.

The ATtiny85 temperature/humidity nodes now continue as their own project,
[AtTinyEnvSensor](https://github.com/jointine-john/AtTinyEnvSensor), including
the coin-cell, PCB and case plans. Both projects share the radio protocol, so
one receiver can hear both kinds of node.

```
 [ATtiny + sensors + STX882] ))) 433.92 MHz OOK ))) [RXB6/SRX882 + ESP32] --WiFi--> MQTT --> Home Assistant
```

## Documentation

| | Page | Contents |
|---|---|---|
| 1 | [Parts](docs/parts.md) | Bill of materials and UK shopping list |
| 2 | [Sensors](docs/sensors.md) | SHT40 vs HTU21D (GY-21), which firmware to build, board notes, troubleshooting |
| 3 | [Wiring](docs/wiring.md) | Schematics, stripboard layout, receiver wiring (RXB6/SRX882), antennas |
| 4 | [Programming the ATtiny85](docs/programming.md) | USBasp, ArduinoISP and SparkFun programmer wiring, troubleshooting |
| 5 | [Building and flashing](docs/building.md) | Build options, node IDs, battery calibration, receiver setup, Home Assistant, loopback and decoder tests |
| 6 | [Power budget](docs/power.md) | Battery life estimate, tuning, measuring sleep current |
| 7 | [Radio protocol](docs/protocol.md) | Frame format, typed readings (T/H, pressure, wind, rain), reliability |
| 8 | [Future plans](docs/future-plans.md) | ATtiny84 port, wind and rain station, receiver pressure sensor, weather display |

## Quick start

```sh
# Sensor node (ATtiny85 on a USBasp; add "-e sht40" for an SHT40 board)
cd sensor-node
pio run -t fuses
PLATFORMIO_BUILD_FLAGS="-DNODE_ID=1" pio run -t upload

# Receiver (ESP32)
cd ../receiver
cp src/secrets.example.h src/secrets.h   # fill in WiFi + MQTT
pio run -t upload && pio device monitor
```

## Repository layout

| Path | What |
|---|---|
| `common/protocol.h` | Over-the-air packet format, shared by both ends |
| `sensor-node/` | ATtiny85 firmware (bare avr-gcc via PlatformIO, no Arduino core) |
| `receiver/` | ESP32 firmware (Arduino via PlatformIO) |
| `test/` | PC test of the Manchester decoder against noisy, jittered signals |
| `docs/` | Documentation |
