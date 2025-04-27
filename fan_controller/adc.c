
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

static uint16_t adc_values[6];

static void readADC(void)
{
    adc_values[0] = read_adc_channel(0);
    adc_values[1] = read_adc_channel(1);
    adc_values[2] = read_adc_channel(2);
    adc_values[3] = read_adc_channel(3);
    adc_values[4] = read_adc_channel(4);
    adc_values[5] = read_adc_channel(6);
}

static uint16_t filter_akk[6];

uint16_t adc_filtered[6];
int16_t temperatures[4];


static void filterAdc()
{
    for (int i = 0; i < 6; i++)
    {
        uint16_t val = filter_akk[i] + adc_values[i] - adc_filtered[i];
        filter_akk[i] = val;
        adc_filtered[i] = val >> 4;
    }
}


static void convertTemperatures()
{
    for (int i = 0; i < 4; i++)
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

