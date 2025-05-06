#pragma once

#include <esp_err.h>
#include <stddef.h>
#include <stdbool.h>

void start_wifi_sta(const char *ssid, const char *pass);

void wifi_init_ap_sta(void);

esp_err_t save_wifi_credentials(const char *ssid, const char *pass);

bool load_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len);

void clear_wifi_credentials();
