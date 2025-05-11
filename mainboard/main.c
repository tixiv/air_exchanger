

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart/uart.h"
#include "rs485_com.h"
#include "adc.h"
#include "rs485_structs.h"


void init_ports(void)
{
	// PA3 : RS485 /RE
	// PA4 : /Stosslueftung in
	// PA5 : /Enable in
	// PA6 : 24V enable
	DDRA = 0x4c;
	PORTA = 0x40;
 
	PORTB = 0x14;
	DDRB = 0xbd;

	// PC0 : 230V fan power relay
	// PC1 : Unused relay
	PORTC = 1;
	DDRC = 0xc3;

	// PD5 : RS485 DE
	PORTD = 5;
	DDRD = 0x3a;
}

mainboard_out_data_t tx_data;
mainboard_in_data_t rx_data;

void set_fan_power(uint8_t state)
{
	if (state)
	{
		PORTC |= 1;
	}
	else
	{
		PORTC &= ~1;
	}
}

int main(){
	init_ports();
	init_adc();
	uart_init();

	_delay_ms(100);

	sei();

	while(1){
		RS485_Buffer_t * buf = update_rs485_com();

		if (buf)
		{
			if (buf->address == 2 && buf->command == 1)
			{
				memcpy(&rx_data, buf->data, sizeof(rx_data));

				set_fan_power(rx_data.power_flags & 0x01);

				memcpy(&tx_data.adc_values, adc_filtered, sizeof(tx_data.adc_values));
				tx_data.inputs = (PINA & 0x30) >> 4;

				rs485_schedule_reply(1, 2, &tx_data, sizeof(tx_data));
			}

		}
		_delay_us(1000);

		update_adc();
	}
}
