#include "wifi_connect.h"

#include <esp_log.h>
#include <esp_netif.h>

extern void example_print_all_netif_ips(const char *prefix);
extern const char *example_ipv6_addr_types_to_str[6];

static const char *TAG = "wifi_connect";

esp_netif_t *wifi_init_ap_sta(void)
{
    // Create default interfaces for STA and AP
    esp_netif_t *wifi_sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = "AirExchanger",
            .ssid_len = strlen("AirExchanger"),
            .channel = 1,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = "", // Will be configured by user later
            .password = ""},
    };

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi in AP+STA mode started");

    return wifi_sta_netif;
}

esp_netif_t *wifi_init_sta(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.if_desc = "netif_sta";
    esp_netif_config.route_prio = 128;
    esp_netif_t *wifi_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    return wifi_sta_netif;
}

esp_err_t my_example_wifi_sta_do_connect(wifi_config_t wifi_config)
{

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    if (0)
    {
        ESP_LOGI(TAG, "Waiting for IP(s)");
#if 0
        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
        vSemaphoreDelete(s_semph_get_ip_addrs);
        s_semph_get_ip_addrs = NULL;
#endif
#if 0
        xSemaphoreTake(s_semph_get_ip6_addrs, portMAX_DELAY);
        vSemaphoreDelete(s_semph_get_ip6_addrs);
        s_semph_get_ip6_addrs = NULL;
#endif
    }
    return ESP_OK;
}
