
#include <stdint.h>

typedef struct  __attribute__((packed))
{
	uint16_t fan_pwm[2];
} fan_in_data_t;

typedef struct  __attribute__((packed))
{
	int16_t temperatures[4];
	uint16_t fan_rpms[2];
} fan_out_data_t;

typedef struct  __attribute__((packed))
{
	uint8_t power_flags; // bit 0: Fan 230V relay on
} mainboard_in_data_t;

typedef struct  __attribute__((packed))
{
	uint16_t adc_values[2];
	uint8_t inputs;   // bit 0: /Stoßlüftung, bit 1: /Freigabe
} mainboard_out_data_t;

typedef struct  __attribute__((packed))
{
	uint8_t heater_duty; // 0-100% dutycycle
} heater_in_data_t;

typedef struct  __attribute__((packed))
{
	int16_t temperatures[2];
	uint8_t heater_duty_readback;
} heater_out_data_t;
