
#include <avr/io.h>
#include <avr/interrupt.h>
#include "fan_rpm.h"

uint8_t pulse_count_1;
uint8_t pulse_count_2;

ISR(INT1_vect)
{
    pulse_count_1++;
}

ISR(INT0_vect)
{
    pulse_count_2 ++;
}

static uint16_t akk_1_0;
static uint16_t akk_1_1;
static uint16_t akk_1_2;

static uint16_t akk_2_0;
static uint16_t akk_2_1;
static uint16_t akk_2_2;

uint16_t fan_rpm_1;
uint16_t fan_rpm_2;


ISR(TIMER0_COMP_vect)
{
    {
        uint16_t akk_1_0_16 = akk_1_0 >> 4;
        akk_1_0 -= akk_1_0_16;

        // if we get one pulse per 10ms that's 6000 RPM. Do half of that to not overflow akkumulators
        akk_1_0 += 3000 * pulse_count_1;
        pulse_count_1 = 0;

        uint16_t akk_1_1_16 = akk_1_1 >> 4;
        akk_1_1 -= akk_1_1_16;
        akk_1_1 += akk_1_0_16;

        akk_1_2 -= akk_1_2 >> 4;
        akk_1_2 += akk_1_1_16;

        // akk_1_2 is now the third order low pass filtered RPM value * 8
        fan_rpm_1 = akk_1_2 >> 3;
    }

    {
        uint16_t akk_2_0_16 = akk_2_0 >> 4;
        akk_2_0 -= akk_2_0_16;

        // if we get one pulse per 10ms that's 6000 RPM. Do half of that to not overflow akkumulators
        akk_2_0 += 3000 * pulse_count_2;
        pulse_count_2 = 0;

        uint16_t akk_2_1_16 = akk_2_1 >> 4;
        akk_2_1 -= akk_2_1_16;
        akk_2_1 += akk_2_0_16;

        akk_2_2 -= akk_2_2 >> 4;
        akk_2_2 += akk_2_1_16;

        // akk_2_2 is now the third order low pass filtered RPM value * 8
        fan_rpm_2 = akk_2_2 >> 3;
    }
}

void init_fan_rpm()
{
    TCCR0 = (1<<WGM01) | 5; // CTC, clk / 1024
    OCR0 = 107; // 11_059_200 Hz / 1024 / 108 = 100 Hz

    TIMSK |= (1<<OCIE0); // Enable compare match interrupt

    MCUCR = 0x0f; // both int0 and int1 trigger on rising edge
    GICR |= (1<<INT0);
    GICR |= (1<<INT1);
}
