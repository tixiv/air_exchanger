#pragma once

#include <esp_err.h>
#include <esp_wifi.h>
#include <stddef.h>
#include <stdbool.h>

void wifi_init_ap_sta(void);

void wifi_init_sta(void);

esp_err_t my_example_wifi_sta_do_connect(wifi_config_t wifi_config);
