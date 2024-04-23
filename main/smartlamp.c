#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi_default.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_netif_types.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "tcpclient.h"

#define WIFI_START_BIT BIT0
#define WIFI_SCAN_BIT BIT1
#define WIFI_CONNECTED_BIT BIT2
#define WIFI_FAIL_BIT BIT3
#define RECEIVED_IP BIT4

#define MQTT_BROKER_ADDRESS "192.168.0.10"
#define REQACK "REQACK"

static const char *TAG = "wifi station";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifiEventGroup;
static ip_event_got_ip_t s_ipAddress;

static void eventHandler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    if (WIFI_EVENT == base && WIFI_EVENT_STA_START == id)
    {
        ESP_LOGI(TAG, "WiFi started\n");
        xEventGroupSetBits(s_wifiEventGroup, WIFI_START_BIT);
    }
    else if (WIFI_EVENT == base && WIFI_EVENT_SCAN_DONE == id)
    {
        ESP_LOGI(TAG, "WiFi scan done\n");
        xEventGroupSetBits(s_wifiEventGroup, WIFI_SCAN_BIT);
    }
    else if (WIFI_EVENT == base && WIFI_EVENT_STA_CONNECTED == id)
    {
        ESP_LOGI(TAG, "WiFi connected\n");
        xEventGroupSetBits(s_wifiEventGroup, WIFI_CONNECTED_BIT);
    }
    else if (WIFI_EVENT == base && WIFI_EVENT_STA_DISCONNECTED == id)
    {
        ESP_LOGI(TAG, "WiFi disconnected\n");
        xEventGroupClearBits(s_wifiEventGroup, WIFI_CONNECTED_BIT);
    }
    else if (IP_EVENT == base && IP_EVENT_STA_GOT_IP == id)
    {
        ip_event_got_ip_t *ipAddress = (ip_event_got_ip_t *)event_data;
        s_ipAddress = *ipAddress;
        ESP_LOGI(TAG, "Got IP address: %d.%d.%d.%d", IP2STR(&s_ipAddress.ip_info.ip));
        xEventGroupSetBits(s_wifiEventGroup, RECEIVED_IP);
    }
    else if (IP_EVENT == base && IP_EVENT_STA_LOST_IP)
    {
        ESP_LOGI(TAG, "Lost IP address\n");
        xEventGroupClearBits(s_wifiEventGroup, RECEIVED_IP);
    }
    else
    {
        ESP_LOGI(TAG, "Unknown event\n");
    }
}

void initiateNetInterface(void)
{
    ESP_LOGI(TAG, "Configuring WiFi STA mode...\n");

    /*
        Following initialization procedure defined by Espressif:
        https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/wifi.html#esp32-wi-fi-station-general-scenario
    */

    // create lwIP stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
}

void configureWifi()
{
    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));

    /*
        register event handler
        esp_event_handler is a legacy function.
        The new one is esp_event_handler_instance_register
    */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventHandler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &eventHandler,
                                                        NULL,
                                                        NULL));

    // configure WiFi
    wifi_config_t wifiConfigSta = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfigSta));

    // start WiFi

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    xEventGroupWaitBits(s_wifiEventGroup,
                        WIFI_START_BIT | WIFI_SCAN_BIT,
                        pdFALSE,
                        pdTRUE,
                        portMAX_DELAY);
    ESP_ERROR_CHECK(esp_wifi_connect());

    xEventGroupWaitBits(s_wifiEventGroup,
                        WIFI_CONNECTED_BIT | RECEIVED_IP,
                        pdFALSE,
                        pdTRUE,
                        portMAX_DELAY);
}

void smartLampMainTask(void *pvParameters)
{
    int tcpSocketFd = tcpCreateClientSocket(MQTT_BROKER_ADDRESS, 8883);
    char recBuffer[256] = {0};
    while (1)
    {
        xEventGroupWaitBits(s_wifiEventGroup,
                            WIFI_CONNECTED_BIT,
                            pdFALSE,
                            pdTRUE,
                            portMAX_DELAY);

        // TODO: Implement an mqtt communication

        ESP_LOGI("smartLampMainTask", "Sending REQACK");
        tcpSendData(tcpSocketFd, REQACK, 7);
        if (-1 != tcpRecvData(tcpSocketFd, recBuffer, 256))
        {
            ESP_LOGI("smartLampMainTask", "Received: %s", recBuffer);
            memset(recBuffer, 0, 256);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ESP_OK != ret)
    {
        ESP_LOGE("NVS", "Error initializing NVS");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifiEventGroup = xEventGroupCreate();
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);

    initiateNetInterface();
    configureWifi();

    xTaskCreate(&smartLampMainTask, "smartLampMainTask", 2048, NULL, 5, NULL);
}
