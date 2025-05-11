

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart/uart.h"
#include "rs485_com.h"
#include "fan_pwm.h"
#include "fan_rpm.h"
#include "adc.h"
#include "rs485_structs.h"


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

fan_out_data_t tx_data;
fan_in_data_t rx_data;

int main(){
	init_ports();
	init_adc();
	uart_init();

	_delay_ms(100);

	init_fan_pwm();
	init_fan_rpm();

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
				tx_data.fan_rpms[0] = fan_rpm_1;
				tx_data.fan_rpms[1] = fan_rpm_2;

				rs485_schedule_reply(1, 4, &tx_data, sizeof(tx_data));
			}

		}
		_delay_us(1000);

		update_adc();
	}
}
