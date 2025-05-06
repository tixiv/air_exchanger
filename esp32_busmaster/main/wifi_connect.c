#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "wifi_connect.h"

static const char *TAG = "wifi_connect";

esp_netif_t *wifi_sta_netif = NULL;

void wifi_connect(void)
{
    // Create default Wi-Fi STA (station) interface
    wifi_sta_netif = esp_netif_create_default_wifi_sta();

    // 💡 Set the hostname before starting Wi-Fi/DHCP
    ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_sta_netif, "air-exchanger"));

    // Wi-Fi init config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Set Wi-Fi mode to STA (station)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Set SSID and password
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "MySSID",
            .password = "MySuperSecretPassword",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Start Wi-Fi and connect
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi started, connecting to network...");

    ESP_ERROR_CHECK(esp_wifi_connect());
}
