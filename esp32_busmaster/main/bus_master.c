
#include "bus_master.h"
#include "../../common/rs485_structs.h"
#include "../../common/rs485_com.h"

#include "driver/uart.h"
#include "stdio.h"

void printHex(uint8_t b)
{
	char sbuf[3];

	itoa(b, sbuf, 16);
	if (sbuf[1] == 0) uart_0_putc('0');
	uart_0_putstr(sbuf);
	uart_0_putc(' ');
}

mainboard_in_data_t mainboard_tx_data;
mainboard_out_data_t mainboard_rx_data;

fan_in_data_t fan_tx_data;
fan_out_data_t fan_rx_data;

heater_in_data_t heater_tx_data;
heater_out_data_t heater_rx_data;

uint8_t device_index;

uint8_t timeout_counter;


void handle_rs485()
{
	static int delay_count;

	if (timeout_counter)
	{
		RS485_Buffer_t *received = update_rs485_com();

		if(received)
		{
			if (received->address == 1 && received->command == 2)
			{
				timeout_counter = 0;
				memcpy(&mainboard_rx_data, received->data, sizeof(mainboard_rx_data));
			}

			if (received->address == 1 && received->command == 4)
			{
				timeout_counter = 0;
				memcpy(&fan_rx_data, received->data, sizeof(fan_rx_data));
			}

			if (received->address == 1 && received->command == 5)
			{
				timeout_counter = 0;
				memcpy(&heater_rx_data, received->data, sizeof(heater_rx_data));
			}
		}
	}
	
	if (timeout_counter == 0)
	{
		if (delay_count++ == 100)
		{
			delay_count = 0;
			timeout_counter = 255;

			switch (device_index)
			{
				case 0: rs485_transmit(2, 1, &mainboard_tx_data, sizeof(mainboard_tx_data)); break;
				case 1: rs485_transmit(4, 1, &fan_tx_data, sizeof(fan_tx_data)); break;
				case 2: rs485_transmit(5, 1, &heater_tx_data, sizeof(heater_tx_data)); break;
			}
			
			device_index++;
			if (device_index == 3)
				device_index = 0;
		}
	}
	else
	{
		timeout_counter--;
		if (timeout_counter == 0)
		{
			printf("Receive Timeout %d\r\n", device_index);
		}
	}
}

void print_temperature(int16_t t)
{
	if (t < 0)
	{
		uart_0_putc('-');
		t = -t;
	}

	printf("%d.%d°C", t/10 , t%10);
}

void print_status(void)
{
	for (uint8_t i=0; i<4; i++)
	{
		printf("T%d: ", i+1);
		print_temperature(fan_rx_data.temperatures[i]);
		printf(", ");
	}

	printf("Fan1: %d, Fan2: %d, Heater: %d, ", fan_tx_data.fan_pwm[0], fan_tx_data.fan_pwm[1], heater_rx_data.heater_duty_readback);

	printf("ADC0: %d, ADC1: %d\r\n", mainboard_rx_data.adc_values[0], mainboard_rx_data.adc_values[1]);
}

#define PC_BUF_SIZE 100


static void uart_0_putc(uint8_t c)
{
  uart_write_bytes(UART_NUM_0, &c, 1);
}

static uint8_t uart_0_getc_nb(uint8_t *c)
{
  return uart_read_bytes(UART_NUM_0, c, 1, 0);
}


void handle_pc(){
	static char buffer[PC_BUF_SIZE];
	static char *pos = buffer;
	static int delay_count;

	uint8_t tmp;
	if(uart_0_getc_nb(&tmp))
	{
		uart_0_putc(tmp);
		
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

			int fan_id;
			int value;

			if (sscanf(buffer, "fan %d %d", &fan_id, &value) == 2 && (fan_id == 1 || fan_id == 2) && value >= 0 && value <= 1023)
			{
				fan_tx_data.fan_pwm[fan_id-1] = value;
			}
			else if (sscanf(buffer, "heater %d", &value) == 1 && value >= 0 && value <= 100)
			{
				heater_tx_data.heater_duty = value;
			}
		}
	}

	if (pos == buffer)
	{
		if (delay_count++ == 10000)
		{
			delay_count = 0;
			print_status();
		}
	}
	else
	{
		delay_count = 0;
	}
}

void handle_bus_master()
{
    handle_rs485();
    handle_pc();
}
