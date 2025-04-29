
#include "conversion_table.h"
#include <avr/io.h>
#include "adc.h"


uint16_t read_adc_channel(uint8_t channel)
{
    ADMUX = channel;

    for (volatile uint8_t delay = 6; delay != 0; delay--);

    ADCSRA |= 0x40;
    while (ADCSRA & 0x40);
    
    return ADCW;  
}

static uint16_t adc_values[2];

static void readADC(void)
{
    adc_values[0] = read_adc_channel(5);
    adc_values[1] = read_adc_channel(6);
}

static uint16_t filter_akk[2];

uint16_t adc_filtered[2];
int16_t temperatures[2];


static void filterAdc()
{
    for (int i = 0; i < 2; i++)
    {
        uint16_t val = filter_akk[i] + adc_values[i] - adc_filtered[i];
        filter_akk[i] = val;
        adc_filtered[i] = val >> 4;
    }
}

static void convertTemperatures()
{
    for (int i = 0; i < 2; i++)
    {
        temperatures[i] = pgm_read_word(&temperature_conversion_table[adc_filtered[i] & 0x3ff]);
    }
}

void update_adc(void)
{
    readADC();
    filterAdc();
    convertTemperatures();
}

void init_adc(void)
{  
  ADMUX = 0;
  ADCSRA = 0xc3;
  while (ADCSRA & 0x40);
}

