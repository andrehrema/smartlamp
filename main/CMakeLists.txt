idf_component_register(SRCS "tcpclient.c" "smartlamp.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_netif esp_event esp_wifi nvs_flash)
