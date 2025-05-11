
#include "bus_master.h"
#include "../../common/rs485_com.h"
#include "ui_values.h"
#include "rs485_uart.h"

#include <stdio.h>
#include <string.h>

mainboard_in_data_t mainboard_tx_data;
mainboard_out_data_t mainboard_rx_data;

fan_in_data_t fan_tx_data;
fan_out_data_t fan_rx_data;

heater_in_data_t heater_tx_data;
heater_out_data_t heater_rx_data;

SemaphoreHandle_t rs485_data_mutex;
struct RS485_data rs485_data;

static void update_fan_rx_data()
{
	if (xSemaphoreTake(rs485_data_mutex, portMAX_DELAY))
	{
		for (int i = 0; i < 4; i++)
		{
			rs485_data.temperatures[i] = fan_rx_data.temperatures[i] / 10.0f;
		}

		for (int i = 0; i < 2; i++)
		{
			rs485_data.fan_rpms[i] = fan_rx_data.fan_rpms[i];
		}

		xSemaphoreGive(rs485_data_mutex);
	}
}

static void update_heater_rx_data()
{
	if (xSemaphoreTake(rs485_data_mutex, portMAX_DELAY))
	{
		rs485_data.temperatures[4] = heater_rx_data.temperatures[0] / 10.0f;
		rs485_data.heater_duty_readback = heater_rx_data.heater_duty_readback;

		xSemaphoreGive(rs485_data_mutex);
	}
}

static void update_mainboard_tx_data()
{
	mainboard_tx_data.power_flags = ui_values.power ? 1:0;
}

static void update_fan_tx_data()
{
	for (int i = 0; i < 2; i++)
	{
		fan_tx_data.fan_pwm[i] = ui_values.power ? (ui_values.fan_speeds[i] * (1023.0f / 100.0f)) : 0;
	}
}

static void update_heater_tx_data()
{
	heater_tx_data.heater_duty = ui_values.power ? ui_values.heater : 0;
}

static uint8_t device_index;
static uint8_t timeout_counter;

static void handle_bus_master()
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
				update_fan_rx_data();
			}

			if (received->address == 1 && received->command == 5)
			{
				timeout_counter = 0;
				memcpy(&heater_rx_data, received->data, sizeof(heater_rx_data));
				update_heater_rx_data();
			}
		}
	}

	if (timeout_counter == 0)
	{
		if (delay_count++ == 2)
		{
			delay_count = 0;
			timeout_counter = 5;

			switch (device_index)
			{
			case 0:
				update_mainboard_tx_data();
				rs485_transmit(2, 1, &mainboard_tx_data, sizeof(mainboard_tx_data));
				break;
			case 1:
				update_fan_tx_data();
				rs485_transmit(4, 1, &fan_tx_data, sizeof(fan_tx_data));
				break;
			case 2:
				update_heater_tx_data();
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

void bus_master_task(void *)
{
	rs485_uart_init();
	while (1)
	{
		handle_bus_master();
		vTaskDelay(1);
	}
}
