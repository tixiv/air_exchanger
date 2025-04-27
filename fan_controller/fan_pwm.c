
#include "fan_pwm.h"
#include <avr/io.h>

void init_fan_pwm_with_mode(uint8_t mode, uint8_t use_timer1)
{
    TCCR1A = 0xa3;
    TCCR1B = 10;

    if (mode == 0)
    {
        PORTB = PORTB | 1;
        PORTB = PORTB & 0xfb;
        PORTB = PORTB | 2;
        PORTB = PORTB & 0xf7;
        if (use_timer1 == 0)
        {
            TCCR1A |= 0x10;
            TCCR1A |= 0x40;
        }
    }
    else if (mode == 1)
    {
        PORTB = PORTB | 1;
        PORTB = PORTB | 4;
        PORTB = PORTB | 2;
        PORTB = PORTB | 8;
        if (use_timer1 == 1)
        {
            TCCR1A |= 0x10;
            TCCR1A |= 0x40;
        }
    }
    else if (mode == 2)
    {
        PORTB = PORTB & 0xfe;
        PORTB = PORTB & 0xfb;
        PORTB = PORTB & 0xfd;
        PORTB = PORTB & 0xf7;
        if (use_timer1 == 0)
        {
            TCCR1A |= 0x10;
            TCCR1A |= 0x40;
        }
    }
    else
    {
        PORTB = PORTB | 1;
        PORTB = PORTB & 0xfb;
        PORTB = PORTB | 1;
        PORTB = PORTB & 0xfb;
        if (use_timer1 == 0)
        {
            TCCR1A |= 0x10;
            TCCR1A |= 0x40;
        }
    }

    OCR1AH = 0;
    OCR1AL = 0;
    OCR1BH = 0;
    OCR1BL = 0;
    return;
}

void init_fan_pwm()
{
    if (PINC & 0x40) {
        init_fan_pwm_with_mode(0, 1);
    }
    else {
        init_fan_pwm_with_mode(2, 1);
    }
}

void set_pwm(uint16_t fan1, uint16_t fan2)
{
    OCR1A = fan1;
    OCR1B = fan2;
}
