
idf_component_register(SRCS "app_main.c" "rs485_com.c" "rs485_uart.c" "bus_master.c" "telnet_server.c" "shell_task.c" "webserver.c" "webserver_spiffs.c" "wifi.c" "wifi_connect.c" "webserver_sse.c" "shell_commands.c" "mqtt_app.c"
                    PRIV_REQUIRES mqtt nvs_flash esp_netif esp_driver_uart shell esp_http_server fatfs spiffs json esp_wifi
                    INCLUDE_DIRS ".")

spiffs_create_partition_image(www ${CMAKE_CURRENT_SOURCE_DIR}/../../esp32-webui/spiffs FLASH_IN_PROJECT)

# This is equivalent to adding WHOLE_ARCHIVE option to the idf_component_register call above:
idf_component_set_property(${COMPONENT_NAME} WHOLE_ARCHIVE TRUE)
