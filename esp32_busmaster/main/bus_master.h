#pragma once

#include "../../common/rs485_structs.h"

extern mainboard_in_data_t mainboard_tx_data;
extern mainboard_out_data_t mainboard_rx_data;

extern fan_in_data_t fan_tx_data;
extern fan_out_data_t fan_rx_data;

extern heater_in_data_t heater_tx_data;
extern heater_out_data_t heater_rx_data;

void handle_bus_master();
