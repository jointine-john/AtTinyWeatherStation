/* 433 MHz OOK transmitter (STX882 / FS1000A) driven by a single data pin. */
#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include "board.h"     /* RADIO_PORT, RADIO_DDR, RADIO_TX_PIN */

void radio_init(void);
/* Sends a len-byte packet (from proto_end) PROTO_REPEATS times. */
void radio_send(const uint8_t *packet, uint8_t len);

#endif
