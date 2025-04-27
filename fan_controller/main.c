

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart/uart.h"
#include "rs485_com.h"
#include "fan_pwm.h"
#include "adc.h"


void init_ports(void)
{
  PORTA = 0xA0;
  DDRA = 0xA0;
  
  PORTB = 3;
  DDRB = 0xbf;
  
  // PC6: Dip Switch 2
  // PC7: Dip Switch 1
  PORTC = 0;
  DDRC = 3;

  PORTD = 3;
  DDRD = 0x72;
}


struct {
	int16_t temperatures[4];
} tx_data;

struct {
	uint16_t fan_pwm[2];
} rx_data;


int main(){
	init_ports();
	init_adc();
	uart_init();

	_delay_ms(100);

	init_fan_pwm();

	sei();

	while(1){
		RS485_Buffer_t * buf = update_rs485_com();

		if (buf)
		{
			if (buf->address == 4 && buf->command == 1)
			{
				memcpy(&rx_data, buf->data, sizeof(rx_data));
				set_pwm(rx_data.fan_pwm[0], rx_data.fan_pwm[1]);

				memcpy(tx_data.temperatures, temperatures, sizeof(temperatures));

				rs485_transmit(1, 4, &tx_data, sizeof(tx_data));
			}

		}
		_delay_us(100);

		update_adc();
	}
}
