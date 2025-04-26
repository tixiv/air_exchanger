

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart/uart.h"
#include "rs485_com.h"
#include "fan_pwm.h"


void init_ports(void)
{
  PORTA = 0xA0;
  DDRA = 0xA0;
  
  PORTB = 3;
  DDRB = 0xbf;
  
  PORTC = 0;
  DDRC = 3;

  PORTD = 3;
  DDRD = 0x72;
}


int main(){
	init_ports();
	uart_init();

	_delay_ms(100);

	init_fan_pwm();

	while(1){
		RS485_Buffer_t * buf = update_rs485_com();

		uint16_t val;
		memcpy(&val, buf->data, 2);

		if (buf)
		{
			switch(buf->command)
			{
				case 1: set_pwm(1, val);
				case 2: set_pwm(1, val);
			}
		}
		_delay_us(100);
	}
}
