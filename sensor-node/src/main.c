/*
 * ATtiny85 / ATtiny84 sensor node.
 *
 * Wakes every INTERVAL_S seconds (via the watchdog), reads the sensor
 * (SHT40 or HTU21D, chosen by the PlatformIO environment) and the
 * supply voltage, transmits one packet over 433 MHz and goes back to
 * power-down sleep. Sleep current is dominated by the watchdog (~4-5 uA).
 *
 * Pinouts for each chip are in board.h. VCC is 2x AA direct, no regulator.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "protocol.h"
#include "board.h"
#include "i2c.h"
#include "sensor.h"
#include "radio.h"

#ifndef NODE_ID
#define NODE_ID 1
#endif

#ifndef INTERVAL_S
#define INTERVAL_S 300
#endif

/* Internal 1.1 V bandgap, nominal. Each chip is within +/-10%; measure VCC
 * with a meter once and adjust this to calibrate the battery reading. */
#ifndef BANDGAP_MV
#define BANDGAP_MV 1100
#endif

#define WDT_16MS 0
#define WDT_8S   (_BV(WDP3) | _BV(WDP0))

EMPTY_INTERRUPT(WDT_vect);

/* Power-down sleep for one watchdog period, then stop the watchdog so it
 * can't fire while we're transmitting. */
static void sleep_wdt(uint8_t prescale)
{
    cli();
    wdt_reset();
    MCUSR &= ~_BV(WDRF);
    WDT_CTRL = _BV(WDCE) | _BV(WDE);
    WDT_CTRL = _BV(WDIE) | prescale;

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
#if defined(BODS) && defined(BODSE)
    sleep_bod_disable();
#endif
    sei();
    sleep_cpu();
    sleep_disable();

    cli();
    WDT_CTRL = _BV(WDCE) | _BV(WDE);
    WDT_CTRL = 0;
    sei();
}

void sleep_16ms(void)
{
    sleep_wdt(WDT_16MS);
}

static uint16_t read_vcc_mv(void)
{
    power_adc_enable();
    ADMUX = ADMUX_BANDGAP;                         /* measure Vbg, VCC as reference */
    ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);  /* 1 MHz / 8 = 125 kHz */
    _delay_ms(2);                                  /* bandgap settling */

    ADCSRA |= _BV(ADSC);                           /* first result is discarded */
    while (ADCSRA & _BV(ADSC))
        ;
    ADCSRA |= _BV(ADSC);
    while (ADCSRA & _BV(ADSC))
        ;
    uint16_t adc = ADC;

    ADCSRA = 0;
    power_adc_disable();
    return adc ? (uint16_t)(((uint32_t)BANDGAP_MV * 1024) / adc) : 0;
}

static uint16_t lfsr_next(uint16_t lfsr)
{
    return (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
}

int main(void)
{
    /* Run at 1 MHz from the 8 MHz RC oscillator regardless of the CKDIV8 fuse:
     * safe down to 1.8 V and low active current. */
    clock_prescale_set(clock_div_8);

    ACSR |= _BV(ACD);       /* analog comparator off */
    ADCSRA = 0;
    power_all_disable();    /* timers, USI, ADC */

    board_init_pins();      /* unused pins driven low so they never float */

    radio_init();
    i2c_init();
    sensor_init();

    uint8_t seq = 0;
    uint16_t lfsr = 0xACE1u ^ ((uint16_t)NODE_ID << 8);
    uint8_t packet[PROTO_MAX_PACKET_LEN];

    for (;;) {
        int16_t temp;
        uint16_t rh;

        uint8_t n = proto_begin(packet, NODE_ID, seq++, read_vcc_mv());
        if (sensor_read(&temp, &rh)) {
            n = proto_put16(packet, n, PROTO_TEMPERATURE, (uint16_t)temp);
            n = proto_put16(packet, n, PROTO_HUMIDITY, rh);
        } else {
            n = proto_put8(packet, n, PROTO_STATUS, PROTO_STATUS_SENSOR_ERROR);
        }
        radio_send(packet, proto_end(packet, n));

        /* Randomly add one extra 8 s period so nodes that started together
         * drift apart instead of colliding every time. */
        lfsr = lfsr_next(lfsr);
        uint16_t wakes = INTERVAL_S / 8 + (lfsr & 1);
        while (wakes--)
            sleep_wdt(WDT_8S);
    }
}
