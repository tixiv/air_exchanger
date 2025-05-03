idf_component_register(SRCS "app_main.c" "rs485_com.c" "rs485_uart.c"
                    PRIV_REQUIRES mqtt nvs_flash esp_netif
                    INCLUDE_DIRS ".")
