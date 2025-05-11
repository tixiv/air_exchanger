
#include "command.h"
#include "bus_master.h"

#include <stdint.h>

int print_status(int, char *[])
{
	for (uint8_t i = 0; i < 5; i++)
	{
		printf("T%d: %f°C,", i + 1, rs485_data.temperatures[i]);
	}

	printf("Fan1: %d, Fan2: %d, Heater: %d, ", fan_tx_data.fan_pwm[0], fan_tx_data.fan_pwm[1], heater_rx_data.heater_duty_readback);

	printf("ADC0: %d, ADC1: %d\r\n", mainboard_rx_data.adc_values[0], mainboard_rx_data.adc_values[1]);

    return 0;
}

SHELL_CMD(status,     (cmdfunc_t)print_status,      "print air exchanger status")
