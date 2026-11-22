#include <avr/io.h>
#include <util/delay.h>
#include "radio.h"
#include "protocol.h"

/* Timing uses busy-wait delays; the Manchester decoder tolerates far more
 * error than the few cycles of loop overhead or the RC oscillator drift. */
static void half_bit(uint8_t level)
{
    if (level)
        RADIO_PORT |= _BV(RADIO_TX_PIN);
    else
        RADIO_PORT &= ~_BV(RADIO_TX_PIN);
    _delay_us(PROTO_HALF_BIT_US);
}

static void send_bit(uint8_t bit)
{
    half_bit(bit);
    half_bit(!bit);
}

static void send_byte(uint8_t byte)
{
    for (uint8_t m = 0x80; m; m >>= 1)
        send_bit((byte & m) ? 1 : 0);
}

void radio_init(void)
{
    RADIO_DDR |= _BV(RADIO_TX_PIN);
    RADIO_PORT &= ~_BV(RADIO_TX_PIN);
}

void radio_send(const uint8_t *packet, uint8_t len)
{
    for (uint8_t r = 0; r < PROTO_REPEATS; r++) {
        for (uint8_t i = 0; i < PROTO_PREAMBLE_LEN; i++)
            send_byte(PROTO_PREAMBLE_BYTE);
        send_byte(PROTO_SYNC_BYTE);
        for (uint8_t i = 0; i < len; i++)
            send_byte(packet[i]);
        /* Ensures the last CRC half-bit is terminated by an edge. */
        send_bit(0);
        RADIO_PORT &= ~_BV(RADIO_TX_PIN);
        if (r + 1 < PROTO_REPEATS)
            _delay_ms(PROTO_REPEAT_GAP_MS);
    }
}
