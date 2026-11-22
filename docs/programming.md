# Programming the ATtiny

ISP uses six pins. They are in different places on the ATtiny85 and the
ATtiny84:

```
           ATtiny85                            ATtiny84
         +----\/----+                       +-----\/-----+
  RESET -| 1      8 |- VCC             VCC -| 1      14 |- GND
        -| 2      7 |- SCK                 -| 2      13 |-
        -| 3      6 |- MISO                -| 3      12 |-
    GND -| 4      5 |- MOSI          RESET -| 4      11 |-
         +----------+                      -| 5      10 |-
                                           -| 6       9 |- SCK
                                      MOSI -| 7       8 |- MISO
                                            +-----------+
```

Easiest is a bare chip on a breadboard with a 100 nF capacitor across VCC and
GND. Build and upload with the PlatformIO environment for your chip (see
[Building and flashing](building.md#sensor-node)); avrdude checks the chip's
signature, so uploading the wrong chip's build fails safely.

## USBasp (default in `platformio.ini`)

10-pin header, looking into the connector with the notch at the top:

```
  MOSI  1 ● ● 2  VCC
   NC   3 ● ● 4  GND
 RESET  5 ● ● 6  GND
  SCK   7 ● ● 8  GND
  MISO  9 ● ● 10 GND
```

| USBasp | ATtiny85 | ATtiny84 |
|---|---|---|
| 1 MOSI | 5 (PB0) | 7 (PA6) |
| 5 RESET | 1 (PB5) | 4 (PB3) |
| 7 SCK | 7 (PB2) | 9 (PA4) |
| 9 MISO | 6 (PB1) | 8 (PA5) |
| 2 VCC | 8 | 1 |
| 4/6/8/10 GND | 4 | 14 |

- **Voltage jumper:** 5 V is fine for a bare chip, or with a GY-21 connected (it
  has its own regulator). Use 3.3 V if a bare SHT40 is already connected, as it
  is only rated to 3.6 V.
- **"cannot set sck period" warning:** older USBasp firmware prints this. It is harmless.
- **Linux USB permissions:** install PlatformIO's udev rules once:
  ```sh
  curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
  sudo udevadm control --reload-rules && sudo udevadm trigger
  ```

## Arduino Uno/Nano as ISP

1. Upload **File → Examples → 11.ArduinoISP → ArduinoISP** to the Arduino.
2. Wire:

   | Arduino | ATtiny85 | ATtiny84 |
   |---|---|---|
   | D10 | 1 (RESET) | 4 (RESET) |
   | D11 | 5 (MOSI) | 7 (MOSI) |
   | D12 | 6 (MISO) | 8 (MISO) |
   | D13 | 7 (SCK) | 9 (SCK) |
   | 5V | 8 | 1 |
   | GND | 4 | 14 |

3. Put a **10 µF capacitor between the Arduino's RESET and GND** (negative leg
   to GND) so avrdude doesn't reset the Arduino. Fit it after uploading
   ArduinoISP.
4. In `sensor-node/platformio.ini`, replace the USBasp `upload_protocol` /
   `upload_flags` lines with:
   ```ini
   upload_protocol = stk500v1
   upload_port = /dev/ttyACM0      ; Uno; a Nano is usually /dev/ttyUSB0
   upload_speed = 19200
   upload_flags =
       -P$UPLOAD_PORT
       -b$UPLOAD_SPEED
   ```

## SparkFun Tiny AVR Programmer

ATtiny85 only: its socket is 8-pin. Plug the chip in and set
`upload_protocol = usbtiny` in `sensor-node/platformio.ini` (remove the
`upload_flags` line). For the ATtiny84, use a USBasp or an Arduino.

## Troubleshooting

If the programmer can't connect, check the wiring first. The chip runs at 1 MHz,
so the ISP clock must stay below 250 kHz. The USBasp is already slowed to about
30 kHz (`-B 32`), and ArduinoISP's default of about 166 kHz is fine.

## Reprogramming a finished sensor in place

The ISP data pins are shared with the sensor and transmitter (ATtiny85:
PB0/PB1/PB2; ATtiny84: PA6/PA5/PA4). Neither interferes with programming, so a 6-pin ISP header on the sensor board lets you reflash
without removing the chip.

**Remove the batteries first**, or set the programmer to 3.3 V and leave its
VCC disconnected. Otherwise the programmer's 5 V back-feeds the AA cells,
which is unsafe for alkalines.
