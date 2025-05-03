
#include <avr/io.h>




ISR(INT0_vect)
{
    
}

ISR(INT1_vect)
{
    
}


ISR(TIMER0_COMP_vect)
{
    
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
