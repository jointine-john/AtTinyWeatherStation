#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"

/* PORT bits for SDA/SCL stay 0, so setting DDR drives the line low and
 * clearing it releases the line to the pull-up. */
#define SDA_LOW()     (I2C_DDR |= _BV(I2C_SDA))
#define SDA_RELEASE() (I2C_DDR &= ~_BV(I2C_SDA))
#define SCL_LOW()     (I2C_DDR |= _BV(I2C_SCL))
#define SCL_RELEASE() (I2C_DDR &= ~_BV(I2C_SCL))
#define SDA_READ()    (I2C_PIN & _BV(I2C_SDA))
#define HALF()        _delay_us(5)

void i2c_init(void)
{
    I2C_PORT &= ~(_BV(I2C_SDA) | _BV(I2C_SCL));
    SDA_RELEASE();
    SCL_RELEASE();
}

void i2c_start(void)
{
    SDA_RELEASE();
    SCL_RELEASE();
    HALF();
    SDA_LOW();
    HALF();
    SCL_LOW();
}

void i2c_stop(void)
{
    SDA_LOW();
    HALF();
    SCL_RELEASE();
    HALF();
    SDA_RELEASE();
    HALF();
}

uint8_t i2c_write(uint8_t byte)
{
    for (uint8_t m = 0x80; m; m >>= 1) {
        if (byte & m)
            SDA_RELEASE();
        else
            SDA_LOW();
        HALF();
        SCL_RELEASE();
        HALF();
        SCL_LOW();
    }
    SDA_RELEASE();
    HALF();
    SCL_RELEASE();
    HALF();
    uint8_t ack = !SDA_READ();
    SCL_LOW();
    return ack;
}

uint8_t i2c_read(uint8_t ack)
{
    uint8_t byte = 0;
    SDA_RELEASE();
    for (uint8_t i = 0; i < 8; i++) {
        HALF();
        SCL_RELEASE();
        HALF();
        byte = (byte << 1) | (SDA_READ() ? 1 : 0);
        SCL_LOW();
    }
    if (ack)
        SDA_LOW();
    HALF();
    SCL_RELEASE();
    HALF();
    SCL_LOW();
    SDA_RELEASE();
    return byte;
}
