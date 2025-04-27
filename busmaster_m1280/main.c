

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "uart/uart.h"
#include "rs485_com.h"

uint8_t rs485_akt;
uint8_t pc_akt;

void printHex(uint8_t b)
{
	char sbuf[3];

	itoa(b, sbuf, 16);
	if (sbuf[1] == 0) uart_0_putc('0');
	uart_0_putstr(sbuf);
	uart_0_putc(' ');
}

struct
{
	uint16_t fan_pwm[2];
} fan_tx_data;

struct
{
	int16_t temperatures[4];
} fan_rx_data;

void handle_rs485()
{
	static int delay_count;

	RS485_Buffer_t *received = update_rs485_com();

	if(received)
	{
		rs485_akt = 1;

		//uart_hexdump(received->bytes, 32);

		if (received->address == 1 && received->command == 4)
		{
			memcpy(&fan_rx_data, received->data, sizeof(fan_rx_data));

			char buf[64];
			sprintf(buf, "%d, %d, %d, %d\r\n", fan_rx_data.temperatures[0], fan_rx_data.temperatures[1], fan_rx_data.temperatures[2], fan_rx_data.temperatures[3]);

			uart_0_putstr(buf);
		}
	}

	if (delay_count++ == 10000)
	{
		delay_count = 0;
		rs485_transmit(4, 1, &fan_tx_data, sizeof(fan_tx_data));
	}
}


#define PC_BUF_SIZE 100

void handle_pc(){
	static char buffer[PC_BUF_SIZE];
	static char *pos = buffer;
	uint8_t tmp;
	if(uart_0_getc_nb(&tmp))
	{
		uart_0_putc(tmp);
		
		pc_akt = 1;
		if ((pos < buffer+PC_BUF_SIZE-1)) //buffer full ?
		{
			*pos++ = tmp;		//no: write character to buffer
		}
		
		if(tmp == '\r')
		{
			*pos = 0;	//terminate line
			pos = buffer;   //reset pointer
			
			/* line received from PC */

			// uart_0_putstr(buffer); //echo to PC

			/*
			int fan_id;
			int value;

			if (sscanf(buffer, "fan %d %d", &fan_id, &value) == 2)
			{
				rs485_transmit(4, fan_id, (uint8_t*)&value, 2);
			}
				*/
		}
	}
}


void handle_leds(){
	static uint8_t led_1_tmr;
	static uint8_t led_2_tmr;
	
	if(rs485_akt){
		rs485_akt = 0;
		led_1_tmr = 200;
	}
	
	if(led_1_tmr != 0){
		PORTG |= (1<<0);
		led_1_tmr--;
	}else{
		PORTG &= ~(1<<0);
	}
	

	if(pc_akt){
		pc_akt = 0;
		led_2_tmr = 200;
	}
	
	if(led_2_tmr != 0){
		PORTG |= (1<<5);
		led_2_tmr--;
	}else{
		PORTG &= ~(1<<5);
	}
}

int main(){
	DDRG = (1<<5) | (1<<0); //LED outputs
	PORTG = (1<<5) | (1<<0); //LEDs on

	PORTD |= 0x04;				//Pullup on Uart1 RXD


	_delay_ms(100);

	uart_1_init();
	uart_0_init();
	
	uart_0_putstr("Hallo\r\n");
	
	while(1){
		handle_rs485();
		handle_pc();
		handle_leds();
		_delay_us(100);
	}
}
