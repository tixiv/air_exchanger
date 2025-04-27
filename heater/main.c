

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart/uart.h"
#include "rs485_com.h"

void init_ports(void)
{
	// PA 0: /230V input X1 3,4
	// PA 1: /230V input X1 1,2
	// PA 2: SSR drive
	PORTA = 0x18;
	DDRA = 4;

	// PB0: LED
	PORTB = 0;
	DDRB = 1;

	// PC0: RS485 DE
	PORTC = 0;
	DDRC = 1;

	PORTD = 0;
	DDRD = 0;
}

struct {
	uint8_t heater_duty_readback;
} tx_data;

struct {
	uint8_t heater_duty; // 0-100% dutycycle
} rx_data;


struct {
	uint16_t fan_pwm[2];
} rx_data_fan;

struct {
	uint8_t power_flags;
} rx_data_main;

void heater_on()
{
	// Heater
	PORTA |= (1<<2);

	// LED
	PORTB |= 1;
}

void heater_off()
{
	// Heater
	PORTA &= ~(1<<2);

	// LED
	PORTB &= ~1;
}

uint8_t timeout_count_rx_heater = 0xff;
uint8_t timeout_count_rx_fan = 0xff;
uint8_t timeout_count_rx_mainboard = 0xff;

uint8_t pwm_count;

void update_heater()
{
	uint8_t pwm_out = 0;
	if (pwm_count < rx_data.heater_duty)
	{
		pwm_out = 1;
	}

	pwm_count++;
	if (pwm_count >= 100)
	{
		pwm_count = 0;
	}

	uint8_t sanity_check = 0;

	if (rx_data_fan.fan_pwm[0] > 200 && // fan commanded running
		(rx_data_main.power_flags & 0x01) && // fan 230V on
		timeout_count_rx_heater < 200 &&
		timeout_count_rx_fan < 200 &&
		timeout_count_rx_mainboard < 200)
	{
		sanity_check = 1;
		tx_data.heater_duty_readback = rx_data.heater_duty;
	}
	else
	{
		tx_data.heater_duty_readback = 0;
	}

	if (sanity_check && pwm_out)
	{
		heater_on();
	}
	else
	{
		heater_off();
	}
}

int main(){
	init_ports();
	uart_init();

	_delay_ms(100);

	sei();

	uint8_t m = 0;

	while(1){
		if (timeout_count_rx_heater < 0xff)
			timeout_count_rx_heater++;
		if (timeout_count_rx_fan < 0xff)
			timeout_count_rx_fan++;
		if (timeout_count_rx_mainboard < 0xff)
			timeout_count_rx_mainboard++;

		RS485_Buffer_t * buf = update_rs485_com();

		if (buf)
		{
			if (buf->address == 5 && buf->command == 1)
			{
				memcpy(&rx_data, buf->data, sizeof(rx_data));

				rs485_transmit(1, 5, &tx_data, sizeof(tx_data));

				timeout_count_rx_heater = 0;
			}

			if (buf->address == 4 && buf->command == 1)
			{
				memcpy(&rx_data_fan, buf->data, sizeof(rx_data_fan));

				timeout_count_rx_fan = 0;
			}

			if (buf->address == 2 && buf->command == 1)
			{
				memcpy(&rx_data_main, buf->data, sizeof(rx_data_main));

				timeout_count_rx_mainboard = 0;
			}
		}

		if(++m == 10)
		{
			update_heater();
		}
		_delay_ms(1);
	}
}
