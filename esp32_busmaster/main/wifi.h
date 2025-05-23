#pragma once

#include <esp_err.h>

void init_wifi_gpios();

esp_err_t save_wifi_credentials(const char *ssid, const char *pass);

void wifi_init();
