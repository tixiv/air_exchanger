#pragma once

#include "../../common/rs485_structs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <stdint.h>

extern mainboard_in_data_t mainboard_tx_data;
extern mainboard_out_data_t mainboard_rx_data;

extern fan_in_data_t fan_tx_data;
extern fan_out_data_t fan_rx_data;

extern heater_in_data_t heater_tx_data;
extern heater_out_data_t heater_rx_data;

void bus_master_task(void *);

extern SemaphoreHandle_t rs485_data_mutex;

struct RS485_data
{
    float temperatures[5]; // in °C
    uint16_t fan_rpms[2];
    uint8_t heater_duty_readback; // 0-100 %
};

extern struct RS485_data rs485_data;
