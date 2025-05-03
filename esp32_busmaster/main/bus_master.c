
#include "bus_master.h"
#include "../../common/rs485_com.h"

#include <stdio.h>
#include <string.h>

mainboard_in_data_t mainboard_tx_data;
mainboard_out_data_t mainboard_rx_data;

fan_in_data_t fan_tx_data;
fan_out_data_t fan_rx_data;

heater_in_data_t heater_tx_data;
heater_out_data_t heater_rx_data;

uint8_t device_index;

uint8_t timeout_counter;

void handle_bus_master()
{
	static int delay_count;

	if (timeout_counter)
	{
		RS485_Buffer_t *received = update_rs485_com();

		if (received)
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
			case 0:
				rs485_transmit(2, 1, &mainboard_tx_data, sizeof(mainboard_tx_data));
				break;
			case 1:
				rs485_transmit(4, 1, &fan_tx_data, sizeof(fan_tx_data));
				break;
			case 2:
				rs485_transmit(5, 1, &heater_tx_data, sizeof(heater_tx_data));
				break;
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
