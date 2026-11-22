# Future plans

Nothing on this page is built yet. This project is heading towards an ATtiny84
wind and rain station reporting to the ESP32 receiver, which will also get its
own pressure sensor and a display.

The ATtiny85 temperature/humidity nodes (coin-cell power, PCB and case plans)
continue in the separate
[AtTinyEnvSensor](https://github.com/jointine-john/AtTinyEnvSensor) project.
Both use the same radio protocol, so one receiver can hear both.

## Goals

1. ~~Port the node firmware to the **ATtiny84**~~, which has the pins a weather
   station needs. **Done:** builds `attiny84_sht40` and `attiny84_htu21d`;
   not yet tested on hardware.
2. Build a **wind and rain station**: wind speed, gust and direction, and
   rainfall from a tipping-bucket gauge, plus temperature and humidity.
3. Have the **ESP32 receiver** record its own temperature, humidity and
   **pressure**.
4. Add a **weather display** to the receiver.

## 1. ATtiny84

The ATtiny85 has only two free pins (PB3, PB4) once I²C and the radio are
connected. A wind and rain station needs four: anemometer, rain gauge, vane
input and a pin to power the vane. The **ATtiny84** has the same AVR core,
the same low sleep current and the same 8 KB flash, with 11 I/O pins in a
14-pin package (DIP-14 for stripboard, SOIC-14 for the PCB). The ATtiny84A is
the current part number; it runs down to 1.8 V at 1 MHz.

The ATtiny85 temperature/humidity nodes continue in the separate
[AtTinyEnvSensor](https://github.com/jointine-john/AtTinyEnvSensor) project.

### Pin plan

The I²C and radio lines go on the ISP pins, as on the ATtiny85, so the board
can still be reprogrammed in place.

| Pin | Port | Use |
|---|---|---|
| 1 | VCC | Supply |
| 2 | PB0 | Anemometer reed switch (PCINT8) |
| 3 | PB1 | Spare |
| 4 | PB3 | RESET (ISP) |
| 5 | PB2 | Spare |
| 6 | PA7 | Spare |
| 7 | PA6 | I²C SDA / ISP MOSI |
| 8 | PA5 | Radio DATA / ISP MISO |
| 9 | PA4 | I²C SCL / ISP SCK |
| 10 | PA3 | Rain gauge reed switch (PCINT3) |
| 11 | PA2 | Spare |
| 12 | PA1 | Wind vane divider power (high only while reading) |
| 13 | PA0 | Wind vane input (ADC0) |
| 14 | GND | Ground |

The anemometer and rain gauge are on different ports, so each gets its own
pin-change interrupt vector and the firmware never has to work out which one
fired.

### Porting the firmware (done)

- **Pin definitions** live in `sensor-node/src/board.h`, chosen by the chip
  the PlatformIO environment builds for. `radio.c`, `i2c.c` and `main.c` use
  its names instead of fixed ports.
- **Register differences** are handled there too: the watchdog control
  register (`WDTCSR` on the ATtiny84, `WDTCR` on the ATtiny85) and the ADC
  setting for the 1.1 V reference used by the battery reading.
- **Build:** `attiny84_sht40` and `attiny84_htu21d` environments in
  `sensor-node/platformio.ini`, with the same fuses as the ATtiny85.
- The ATtiny85 builds are byte-for-byte unchanged by the port.
- Still to do: flash an ATtiny84, check it reports temperature, humidity and
  battery voltage, and measure its sleep current (expect about 5 µA).

## 2. Wind and rain station

### Hardware

- **Sensors:** a hobby weather-meter kit, e.g. SparkFun SEN-15901 (or the
  Misol parts it is based on). It has a cup anemometer, a wind vane and a
  tipping-bucket rain gauge, all passive reed switches and resistors, on
  RJ11 plugs. Check the kit's datasheet for the RJ11 pin-out.
- **Temperature/humidity:** an SHT40 in a separate radiation shield, so the
  station reports T/H too.
- **MCU and radio:** ATtiny84 (section 1) and STX882.
- **Mounting:** on a mast in the open, so check the radio range to the
  receiver from there.

### Wind speed

- The anemometer closes a reed switch once per revolution; for the SparkFun
  kit, one closure per second is 2.4 km/h (0.667 m/s).
- A pin-change interrupt counts closures. It wakes the ATtiny from power-down
  for a few microseconds per pulse, so counting costs almost nothing.
- The watchdog wakes the node every second. Each second's count goes into a
  3-second rolling sum; the highest one in the interval is the **gust**
  (3 s is the standard gust averaging time). The total over the interval
  gives the **average speed**.
- **Pull-up:** the cups can stop with the switch closed, so the internal
  pull-up (~35 kΩ, ~85 µA when closed) wastes too much. Use an external
  1 MΩ pull-up (3 µA worst case) with 1 nF to ground for debouncing
  (1 ms time constant, fast enough for gale-force pulse rates), plus a short
  software lock-out.
- **Accuracy:** the watchdog's timing is only accurate to about ±10%, so wind
  speed is too. Add a calibration constant (`WDT_CAL`), measured once per
  chip with a scope or by timing many watchdog periods, as `BANDGAP_MV`
  already does for the battery reading.

### Wind direction

- The vane switches in one of 16 resistor combinations. With a 10 kΩ resistor
  from PA1, PA0 reads one of 16 voltages; a lookup table maps the ADC reading
  to a direction (the table is in the kit datasheet).
- PA1 powers the divider for about a millisecond per reading. Left on, it
  would draw up to ~300 µA.
- Sample once per second. Report the vector average of the interval's
  samples (16-entry sine/cosine table, integer maths), or the most common of
  the 16 positions as a simpler first version.
- Sent as deci-degrees, so 16 positions come out as multiples of 22.5°.

### Rain

- The bucket tips once per 0.2794 mm of rain (SparkFun kit) and closes a reed
  switch briefly as it passes.
- A pin-change interrupt counts tips into a running 16-bit count. The node
  sends that count with the bucket size (279 µm), as the protocol already
  defines. A lost packet loses no rain; Home Assistant adds it up as a
  `total_increasing` sensor.
- Same 1 MΩ pull-up as the anemometer, with 10 nF (10 ms) debounce and a
  software lock-out of ~100 ms, since contacts can bounce for several
  milliseconds as the bucket tips.

### Power

A station frame with T/H, wind (speed, gust, direction) and rain has 24 body
bytes, 31 bytes on air, about 250 ms per frame.

| Reporting interval | Transmitter | Total (approx.) | 2×AA lithium (~3000 mAh) |
|---|---|---|---|
| 5 minutes | ~165 mAh/year | ~230 mAh/year | Shelf life is the limit |
| 1 minute | ~820 mAh/year | ~890 mAh/year | ~3 years |

The 1-second watchdog ticks and pulse counting add only a few µA. Wind
changes quickly, so a 1-minute interval is worth considering here: 2×AA
lithium cells (Energizer L91, good in the cold) still last years. A coin cell
is not suitable: it sags badly under the transmitter's ~15 mA bursts, and the
station's frames are nearly twice as long as a T/H node's. A small
solar panel with a supercapacitor is an option if batteries in a mast-top
box are awkward to change.

### Steps

1. Flash the ATtiny84 build (section 1) and check temperature, humidity and
   battery reach the receiver.
2. Add anemometer and rain counting on a breadboard, then check pulse counts
   against hand-turned cups and a measured volume of water poured through the
   gauge.
3. Add the vane, build the ADC lookup table from real readings, and check all
   16 positions.
4. Calibrate the watchdog (`WDT_CAL`) and anemometer factor.
5. Use the receiver's loopback test meanwhile: it already sends wind and rain
   readings, so the Home Assistant side can be set up in advance.
6. Design a station PCB (ATtiny84, SHT40 header, two RJ11 jacks, pull-ups and
   debounce capacitors, AA holder) and a mast-mount case.

## 3. Receiver sensor (temperature, humidity, pressure)

The receiver is mains-powered, so power use doesn't matter here.

### Sensor choice

- **BME280** (recommended): temperature, humidity and pressure in one I²C
  chip, on cheap breakout boards. Buy from a reputable seller: many boards
  sold as "BME280" carry a BMP280, which has no humidity sensor. The firmware
  should check the chip ID (0x60 for BME280, 0x58 for BMP280) and log a clear
  error.
- **Alternative:** SHT40 for temperature and humidity plus a BMP280/BMP390 for
  pressure, on the same I²C bus. More accurate humidity (±1.8% vs ±3% RH), and
  it matches the nodes' sensor, at the cost of a second board.

### Wiring

- I²C on the ESP32 defaults: **SDA → GPIO21, SCL → GPIO22**, VCC → 3V3,
  GND → GND. GPIO4 stays the 433 MHz receiver input.
- Most BME280 boards already have I²C pull-ups.
- **Self-heating:** the ESP32 and its WiFi radio warm the board by a few °C.
  Mount the sensor on a short lead outside or below the receiver's case, away
  from the ESP32 and any regulator.

### Firmware

- Read the sensor every 60 s, in forced mode so it sleeps between readings
  (less self-heating).
- Publish over the existing MQTT connection as its own Home Assistant device
  (e.g. "Receiver"), with Home Assistant auto-discovery like the nodes:
  temperature (°C), humidity (%) and pressure (hPa, device class
  `atmospheric_pressure`).
- Optionally report sea-level pressure as well, using a configured altitude
  (in `secrets.h`), so readings match weather forecasts.
- Use an existing library (e.g. Adafruit BME280) via `lib_deps` in
  `receiver/platformio.ini`.
- Keep it optional: if no sensor answers at start-up, log it and carry on
  receiving, so receivers without a sensor still work.

## 4. Weather display

A small screen on the receiver showing what it already knows: every node's
latest readings and its own pressure sensor.

### Display choice

| Display | Notes |
|---|---|
| **ILI9341 / ST7789 TFT**, 2–3.2", SPI (recommended) | Colour, fast to redraw, room for tiles per room, a wind compass, rain totals and small trend graphs. Libraries: TFT_eSPI or LovyanGFX |
| SSD1306 OLED, 0.96–1.3", I²C | Cheapest; a few lines of text. Shares the I²C bus with the BME280. Burns in if the layout never moves |
| E-paper, 2.9–4.2", SPI | Looks like a printed weather station, readable in sunlight. Slow refresh (2–4 s); fine for updates every few minutes |
| "Cheap Yellow Display" (ESP32-2432S028) | ESP32 and 2.8" touch TFT on one board. Few free pins: check one can take the 433 MHz receiver's DATA line |

### Don't starve the radio decoder

The receiver captures radio edges in an interrupt into a ring buffer of 512
pulses, about one transmission's worth, which `loop()` then decodes. If drawing
blocks `loop()` for too long, packets are lost:

- A TFT redraw takes tens of milliseconds: fine.
- An e-paper refresh blocks for 2–4 s with typical libraries: it would drop
  packets.

Run the display in its own FreeRTOS task on the ESP32's other core, fed from a
copy of the latest readings, so decoding never waits for the screen.

### Pins

- GPIO4 is the radio input and GPIO5 the loopback test output. Many SPI
  display examples use GPIO5 for chip select; use another pin, e.g. GPIO15.
- I²C (OLED, BME280): GPIO21/22. SPI: GPIO18 (SCK), GPIO23 (MOSI), plus CS,
  DC and reset/backlight pins.

### What to show

- Temperature and humidity per node.
- Pressure with a rising/falling trend arrow.
- Wind speed, gust and direction on a compass.
- Rain today (from the running tip count).
- Warnings for low battery or nodes that have gone quiet.
- Optionally, a forecast from Home Assistant, via an MQTT topic the receiver
  subscribes to.

## Open questions

- Wind and rain: which kit? How often should the station report (1 or
  5 minutes)? AA lithium or solar?
- Where will the mast go, and is the radio range to the receiver enough?
- Receiver sensor: BME280 alone, or SHT40 + BMP280/BMP390? Indoors with the
  receiver, or on a lead outside?
- Display: TFT, OLED or e-paper? Built into the receiver, or a second ESP32
  that only listens to MQTT?
