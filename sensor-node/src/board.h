/*
 * Per-chip pin and register definitions, selected by the -mmcu the
 * PlatformIO environment builds for. Everything else in the firmware is
 * shared between the ATtiny85 and the ATtiny84.
 *
 * On both chips the I2C and radio lines sit on the ISP pins (SDA = MOSI,
 * SCL = SCK, radio DATA = MISO), so a finished node can be reprogrammed in
 * place; the transmitter just chirps during upload.
 */
#ifndef BOARD_H
#define BOARD_H

#include <avr/io.h>

#if defined(__AVR_ATtiny85__)

/*
 * ATtiny85 (DIP-8 / SOIC-8): temperature/humidity proof of concept.
 *   1 PB5 RESET  - ISP only
 *   2 PB3        - unused (driven low)
 *   3 PB4        - unused (driven low)
 *   4 GND
 *   5 PB0        - sensor SDA  / ISP MOSI
 *   6 PB1        - radio DATA  / ISP MISO
 *   7 PB2        - sensor SCL  / ISP SCK
 *   8 VCC
 */
#define BOARD_NAME        "ATtiny85"

#define I2C_PORT          PORTB
#define I2C_DDR           DDRB
#define I2C_PIN           PINB
#define I2C_SDA           PB0
#define I2C_SCL           PB2

#define RADIO_PORT        PORTB
#define RADIO_DDR         DDRB
#define RADIO_TX_PIN      PB1

#define WDT_CTRL          WDTCR

/* ADMUX for measuring the 1.1 V bandgap with VCC as the reference. */
#define ADMUX_BANDGAP     (_BV(MUX3) | _BV(MUX2))

/* Unused pins as outputs low so they never float. Must run before
 * radio_init() and i2c_init(), which set up their own pins. */
static inline void board_init_pins(void)
{
    DDRB = _BV(PB3) | _BV(PB4);
    PORTB = 0;
}

#elif defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny84A__)

/*
 * ATtiny84 / ATtiny84A (DIP-14 / SOIC-14): weather station.
 * Pins marked "station" are reserved for the wind and rain hardware
 * (see docs/future-plans.md) and driven low until that code exists.
 *   1 VCC
 *   2 PB0        - anemometer reed switch (station)
 *   3 PB1        - unused
 *   4 PB3 RESET  - ISP only
 *   5 PB2        - unused
 *   6 PA7        - unused
 *   7 PA6        - sensor SDA  / ISP MOSI
 *   8 PA5        - radio DATA  / ISP MISO
 *   9 PA4        - sensor SCL  / ISP SCK
 *  10 PA3        - rain gauge reed switch (station)
 *  11 PA2        - unused
 *  12 PA1        - wind vane divider power (station)
 *  13 PA0        - wind vane input, ADC0 (station)
 *  14 GND
 */
#define BOARD_NAME        "ATtiny84"

#define I2C_PORT          PORTA
#define I2C_DDR           DDRA
#define I2C_PIN           PINA
#define I2C_SDA           PA6
#define I2C_SCL           PA4

#define RADIO_PORT        PORTA
#define RADIO_DDR         DDRA
#define RADIO_TX_PIN      PA5

#define WDT_CTRL          WDTCSR

/* ADMUX for measuring the 1.1 V bandgap with VCC as the reference. */
#define ADMUX_BANDGAP     (_BV(MUX5) | _BV(MUX0))

/* Unused pins as outputs low so they never float. Must run before
 * radio_init() and i2c_init(), which set up their own pins. */
static inline void board_init_pins(void)
{
    DDRA = _BV(PA0) | _BV(PA1) | _BV(PA2) | _BV(PA3) | _BV(PA7);
    PORTA = 0;
    DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);
    PORTB = 0;
}

#else
#error "Unsupported MCU: add its pins to board.h"
#endif

#endif
