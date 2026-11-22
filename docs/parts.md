# Parts

## Bill of materials (per sensor)

| Part | Notes |
|---|---|
| ATtiny85-20PU (DIP-8) or ATtiny85V | Runs at 1 MHz, fine down to 1.8 V |
| …or ATtiny84A-PU (DIP-14) | For the weather station: same firmware, more pins. Build an `attiny84_*` environment |
| SHT40 breakout | Must have SDA/SCL pull-ups (~10 kΩ); Adafruit/generic boards do. **Remove or cut any power LED and regulator** |
| …or GY-21 (HTU21D/SHT21) breakout | Proof-of-concept sensor. Has pull-ups and a ~1 µA regulator, no LED, so it works as-is. Build the `htu21d` environment. See [Sensors](sensors.md) |
| STX882 433 MHz transmitter | Rated 1.2–6 V, works well at 3 V. FS1000A also works but loses range below ~3 V |
| 17.3 cm wire | Quarter-wave antenna |
| 100 nF ceramic | Decoupling, across ATtiny VCC/GND |
| 10 µF ceramic | Across the battery, helps during TX bursts |
| 2×AA holder | Lithium AA (Energizer L91) recommended outdoors/cold |

Receiver: any ESP32 dev board + RXB6 (superheterodyne, much better than the
XY-MK-5V) + 17.3 cm antenna.

## Shopping list (UK)

Links checked November 2026. Stock and prices change, so check before ordering.
Generic modules are linked as eBay UK searches because individual listings expire.

### Per sensor

| Part | Qty | Where |
|---|---|---|
| ATtiny85-20PU (DIP-8) | 1 | [RS 133-1672](https://uk.rs-online.com/web/p/microcontrollers/1331672) (pack of 5, ~£1.25 each), [Farnell 1455162](https://uk.farnell.com/microchip/attiny85-20pu/mcu-8bit-attiny-20mhz-dip-8/dp/1455162) |
| …or ATtiny85V-10PU (rated down to 1.8 V) | 1 | [Farnell 1455165](https://uk.farnell.com/microchip/attiny85v-10pu/mcu-8bit-attiny-10mhz-dip-8/dp/1455165), [DigiKey UK](https://www.digikey.co.uk/en/products/detail/microchip-technology/ATTINY85V-10PU/735471) |
| 8-pin DIP socket | 1 | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=8+pin+DIP+IC+socket) |
| …or ATtiny84A-PU (DIP-14) and a 14-pin DIP socket | 1 | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=ATtiny84A-PU), [eBay UK socket](https://www.ebay.co.uk/sch/i.html?_nkw=14+pin+DIP+IC+socket) |
| …or GY-21 (HTU21D) breakout, proof of concept | 1 | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=GY-21+HTU21D) |
| SHT40 breakout | 1 | [The Pi Hut – Adafruit 4885](https://thepihut.com/products/adafruit-sensirion-sht40-temperature-humidity-sensor-stemma-qt-qwiic), [Pimoroni – Adafruit 4885](https://shop.pimoroni.com/products/adafruit-sensirion-sht40-temperature-humidity-sensor-stemma-qt-qwiic), [The Pi Hut – DFRobot Fermion](https://thepihut.com/products/fermion-sht40-humidity-temperature-sensor-breakout) |
| STX882 433 MHz transmitter | 1 | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=STX882), [Tindie – STX882 + SRX882 kit](https://www.tindie.com/products/icstation/stx882srx882-433mhz-transceiver8251/) |
| 2×AA holder with switch and leads | 1 | [CPC BT06555](https://cpc.farnell.com/multicomp-pro/mp000375/battery-box-2x-aa-switched-wired/dp/BT06555), [CPC BT06560 (waterproof)](https://cpc.farnell.com/pro-power/pp002109/battery-box-2x-aa-waterproof-wired/dp/BT06560), [The Pi Hut](https://thepihut.com/products/battery-holder-2xaa-with-cover-and-switch) |
| 100 nF ceramic capacitor | 1 | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=100nF+ceramic+capacitor+through+hole) |
| 10 µF ceramic capacitor | 1 | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=10uF+ceramic+capacitor+through+hole) |
| Lithium AA (Energizer L91), for outdoor/cold use | 2 | Any supermarket or [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=Energizer+L91+lithium+AA) |
| Stripboard, solid-core wire (also used for the 17.3 cm antenna) | – | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=stripboard+veroboard) |

The Adafruit SHT40 board has a power LED. Cut its jumper or remove it, or it
will dominate the sleep current.

### Receiver (one)

| Part | Where |
|---|---|
| ESP32 dev board (classic ESP32-WROOM-32, matches `board = esp32dev`) | [The Pi Hut – Espressif ESP32 Developer Edition](https://thepihut.com/products/espressif-esp32-development-board-developer-edition), [The Pi Hut – Olimex ESP32-DevKit-LiPo](https://thepihut.com/products/olimex-esp32-devkit-lipo-esp32-development-board), or a generic board from [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=ESP32+WROOM+32+devkit) |
| RXB6 433 MHz superheterodyne receiver | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=RXB6+433MHz) |

An ESP32-S2, S3 or C3 board also works if you change `board` in
`receiver/platformio.ini`. The SRX882 from the STX882 kit works in place of
the RXB6, but its CS (enable) pin must be tied high.

### Programmer (one)

| Part | Where |
|---|---|
| USBasp | [38-3d.co.uk](https://38-3d.co.uk/products/usbasp-isp-programmer), [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=USBasp+programmer) (choose one with the slow-clock JP3 jumper or recent firmware) |
| 10-pin to 6-pin ISP adapter | [eBay UK](https://www.ebay.co.uk/sch/i.html?_nkw=10+pin+to+6+pin+ISP+adapter) |
| …or SparkFun Tiny AVR Programmer (ATtiny85 socket, no wiring needed; set `upload_protocol = usbtiny`) | [The Pi Hut](https://thepihut.com/products/sparkfun-tiny-avr-programmer) |

You don't need a programmer if you already have an Arduino Uno or Nano. See
[Programming the ATtiny](programming.md).
