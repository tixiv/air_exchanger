

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


void receive_from_rs485()
{
	RS485_Buffer_t *buff = update_rs485_com();

	if(buff)
	{
		rs485_akt = 1;

		uart_hexdump(buff->bytes, 32);

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

			uart_0_putstr(buffer); //echo to PC

			int fan_id;
			int value;

			if (sscanf(buffer, "fan %d %d", &fan_id, &value) == 2)
			{
				rs485_transmit(4, fan_id, (uint8_t*)&value, 2);
			}
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
		receive_from_rs485();
		handle_pc();
		handle_leds();
		_delay_us(100);
	}
}
