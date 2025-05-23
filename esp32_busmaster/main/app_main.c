/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "bus_master.h"
#include "telnet_server.h"
#include "webserver.h"
#include "wifi.h"

#include <esp_system.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <mdns.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

static const char *TAG = "app_main";

static void my_mdns_init(void)
{
    
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("air-exchanger"));
    ESP_ERROR_CHECK(mdns_instance_name_set("Air Exchanger Control mDNS"));
    ESP_ERROR_CHECK(mdns_service_add("Air Exchanger mDNS", "_http", "_tcp", 80, NULL, 0));

    ESP_LOGI(TAG, "mDNS service started");
}

struct {
    bool simulate_rs485;
}my_config;

static void loadConfig()
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("config", NVS_READONLY, &nvs);
    if (err != ESP_OK) return;

	int32_t value;

    if (nvs_get_i32(nvs, "simulate_rs485", &value) != ESP_OK)
    {
        nvs_close(nvs);
        return;
    }

    my_config.simulate_rs485 = value ? true : false;

    nvs_close(nvs);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    init_wifi_gpios();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    loadConfig();

    rs485_data_mutex = xSemaphoreCreateMutex();

    if (my_config.simulate_rs485)
    {
        // TODO: actually simulate. just don't run RS485 bus master for now.
    }
    else
    {
        xTaskCreate(bus_master_task, "bus_master", 2048, NULL, 22, NULL);
    }

    wifi_init();

    // After Wi-Fi connects:
    my_mdns_init();
    
    xTaskCreate(telnet_server_task, "telnet_server", 4096, NULL, 5, NULL);

    start_webserver();
}
