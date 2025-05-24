
#include "wifi.h"
#include "wifi_connect.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_event.h>
#include <esp_wifi.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <esp_err.h>
#include <esp_log.h>

extern const char *example_ipv6_addr_types_to_str[6];

#define WIFI_RESET_BUTTON_GPIO 13
#define WIFI_LED_GPIO 14

static const char *TAG = "WifiHandler";

static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static SemaphoreHandle_t s_semph_get_ip6_addrs = NULL;

void init_wifi_gpios()
{
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << WIFI_RESET_BUTTON_GPIO,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};
        gpio_config(&io_conf);
    }
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << WIFI_LED_GPIO),   // Pin 14
            .mode = GPIO_MODE_OUTPUT,             // Output mode
            .pull_up_en = GPIO_PULLUP_DISABLE,    // No pull-up
            .pull_down_en = GPIO_PULLDOWN_DISABLE,// No pull-down
            .intr_type = GPIO_INTR_DISABLE        // No interrupts
        };
        gpio_config(&io_conf);
    }
}

esp_err_t save_wifi_credentials(const char *ssid, const char *pass)
{
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("wifi_creds", NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "pass", pass));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
    return ESP_OK;
}

static bool load_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len)
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("wifi_creds", NVS_READONLY, &nvs);
    if (err != ESP_OK)
        return false;

    if (nvs_get_str(nvs, "ssid", ssid, &ssid_len) != ESP_OK ||
        nvs_get_str(nvs, "pass", pass, &pass_len) != ESP_OK)
    {
        nvs_close(nvs);
        return false;
    }

    nvs_close(nvs);
    return true;
}

static void clear_wifi_credentials()
{
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("wifi_creds", NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_erase_all(nvs));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
}

static void check_reset_button()
{
    if (gpio_get_level(WIFI_RESET_BUTTON_GPIO) == 0)
    {
        ESP_LOGW(TAG, "Reset button held down, clearing credentials...");
        clear_wifi_credentials();
    }
}



/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int retryCounter;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi STA disconnected event");

        wifi_event_sta_disconnected_t *disconn = event_data;
        if (disconn->reason == WIFI_REASON_ROAMING)
        {
            ESP_LOGD(TAG, "station roaming, do nothing");
            return;
        }

        ESP_LOGI(TAG, "Wi-Fi disconnected %d", disconn->reason);

        if (retryCounter)
        {
            retryCounter--;
            ESP_LOGI(TAG, "trying to reconnect...");
            esp_wifi_connect();
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            gpio_set_level(WIFI_LED_GPIO, 0); // LED OFF
        }
    }
    else if (event_base == IP_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        esp_netif_create_ip6_linklocal((esp_netif_t *)arg);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        gpio_set_level(WIFI_LED_GPIO, 1); // LED ON
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6)
    {
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;

        esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
        ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
                 IPV62STR(event->ip6_info.ip), example_ipv6_addr_types_to_str[ipv6_type]);
    }
}

static void register_event_handlers(esp_netif_t *sta_netif)
{
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, sta_netif, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &event_handler, sta_netif, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, sta_netif, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_GOT_IP6, &event_handler, sta_netif, NULL));
}

static esp_err_t my_example_wifi_sta_do_disconnect(void)
{
    return esp_wifi_disconnect();
}

void wifi_init()
{
    s_semph_get_ip_addrs = xSemaphoreCreateBinary();
    s_semph_get_ip6_addrs = xSemaphoreCreateBinary();
    s_wifi_event_group = xEventGroupCreate();

    check_reset_button(); // wipe config if button is pressed

    char ssid[32] = {0};
    char pass[64] = {0};
    if (load_wifi_credentials(ssid, sizeof(ssid), pass, sizeof(pass)))
    {
        ESP_LOGI(TAG, "Found saved credentials, connecting to %s...", ssid);

        esp_netif_t *sta_netif = wifi_init_sta();

        register_event_handlers(sta_netif);

        wifi_config_t wifi_config = {
            .sta = {
                .scan_method = WIFI_ALL_CHANNEL_SCAN, // this can also be WIFI_FAST_SCAN
                .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
                .threshold.rssi = -127,
                .threshold.authmode = WIFI_AUTH_OPEN, // could also be WIFI_AUTH_WPA2_PSK or a couple of other modes
            },
        };

        strcpy((char *)wifi_config.sta.ssid, ssid);
        strcpy((char *)wifi_config.sta.password, pass);

        my_example_wifi_sta_do_connect(wifi_config);
    }
    else
    {
        ESP_LOGW(TAG, "No saved credentials, starting in AP mode");
        esp_netif_t *sta_netif = wifi_init_ap_sta();
        register_event_handlers(sta_netif);
    }
}