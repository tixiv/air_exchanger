
#include "command.h"
#include "bus_master.h"

#include <nvs_flash.h>

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

int nvs_get_str_cmd(int argc, char *argv[])
{
	if (argc < 3) goto usage;

	nvs_handle_t nvs;
    esp_err_t err = nvs_open(argv[1], NVS_READONLY, &nvs);
    if (err != ESP_OK)
	{
		printf("Namespcace \"%s\" not found\n", argv[1]);
        return 1;
	}

	char value[128];
	unsigned int value_length = sizeof(value);

    if (nvs_get_str(nvs, argv[2], value, &value_length) != ESP_OK)
    {
		printf("Key \"%s\" not found\n", argv[2]);
        nvs_close(nvs);
        return 1;
    }

	printf("%s::%s = \"%s\"\n", argv[1], argv[2], value);

    nvs_close(nvs);
    return 0;

usage:
	printf("Usage: %s <namespace> <key>\n", argv[0]);
	return 1;
}

int nvs_set_str_cmd(int argc, char *argv[])
{
	if (argc < 4) goto usage;

	nvs_handle_t nvs;
    esp_err_t err = nvs_open(argv[1], NVS_READWRITE, &nvs);
    if (err != ESP_OK)
	{
		printf("Namespcace \"%s\" not found\n", argv[1]);
        return 1;
	}

    if (nvs_set_str(nvs, argv[2], argv[3]) != ESP_OK)
    {
		printf("Couldn't set value\n");
        nvs_close(nvs);
        return 1;
    }

    if (nvs_commit(nvs)	!= ESP_OK)
    {
		printf("Couldn't commit to nvs\n");
        nvs_close(nvs);
        return 1;
    }

	printf("%s::%s = \"%s\"\n", argv[1], argv[2], argv[3]);

    nvs_close(nvs);
    return 0;

usage:
	printf("Usage: %s <namespace> <key> <value>\n", argv[0]);
	return 1;
}

int nvs_get_int_cmd(int argc, char *argv[])
{
	if (argc < 3) goto usage;

	nvs_handle_t nvs;
    esp_err_t err = nvs_open(argv[1], NVS_READONLY, &nvs);
    if (err != ESP_OK)
	{
		printf("Namespcace \"%s\" not found\n", argv[1]);
        return 1;
	}

	int32_t value;

    if (nvs_get_i32(nvs, argv[2], &value) != ESP_OK)
    {
		printf("Key \"%s\" not found\n", argv[2]);
        nvs_close(nvs);
        return 1;
    }

	printf("%s::%s = \"%ld\"\n", argv[1], argv[2], value);

    nvs_close(nvs);
    return 0;

usage:
	printf("Usage: %s <namespace> <key>\n", argv[0]);
	return 1;
}

int nvs_set_int_cmd(int argc, char *argv[])
{
	if (argc < 4) goto usage;

	nvs_handle_t nvs;
    esp_err_t err = nvs_open(argv[1], NVS_READWRITE, &nvs);
    if (err != ESP_OK)
	{
		printf("Namespcace \"%s\" not found\n", argv[1]);
        return 1;
	}

	char *endp;
	int32_t value = strtol(argv[3], &endp, 0);
	if (*endp != 0) goto usage;

    if (nvs_set_i32(nvs, argv[2], value) != ESP_OK)
    {
		printf("Couldn't set value\n");
        nvs_close(nvs);
        return 1;
    }

    if (nvs_commit(nvs)	!= ESP_OK)
    {
		printf("Couldn't commit to nvs\n");
        nvs_close(nvs);
        return 1;
    }

	printf("%s::%s = \"%s\"\n", argv[1], argv[2], argv[3]);

    nvs_close(nvs);
    return 0;

usage:
	printf("Usage: %s <namespace> <key> <value>\n", argv[0]);
	return 1;
}

SHELL_CMD(status,      print_status,      "print air exchanger status")
SHELL_CMD(nvs_get_str, nvs_get_str_cmd,   "read string from nvs")
SHELL_CMD(nvs_set_str, nvs_set_str_cmd,   "store string in nvs")
SHELL_CMD(nvs_get_int, nvs_get_int_cmd,   "read int32 from nvs")
SHELL_CMD(nvs_set_int, nvs_set_int_cmd,   "store int32 in nvs")
