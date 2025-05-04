idf_component_register(SRCS "app_main.c" "rs485_com.c" "rs485_uart.c" "bus_master.c" "telnet_server.c" "shell_task.c" "webserver.c"
                    PRIV_REQUIRES mqtt nvs_flash esp_netif esp_driver_uart shell esp_http_server
                    INCLUDE_DIRS ".")
