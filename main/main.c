#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#define TAG "WIFI_SNIFFER"
#define BUZZER_PIN GPIO_NUM_2



void sound_alarm() {
    esp_rom_gpio_pad_select_gpio(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(BUZZER_PIN, 0);
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MISC) {
        wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*) buff;
        wifi_pkt_rx_ctrl_t rx_ctrl = pkt->rx_ctrl;
        uint8_t *payload = pkt->payload;

        if (payload[0] == 0xC0 && payload[1] == 0x00) {
            uint8_t* source_mac = payload + 10;
            ESP_LOGI(TAG, "Deauth packet detected from MAC address: %02X:%02X:%02X:%02X:%02X:%02X on channel %d",
                     source_mac[0], source_mac[1], source_mac[2], source_mac[3], source_mac[4], source_mac[5], rx_ctrl.channel);
            sound_alarm();
        }

        if (payload[0] == 0x30 && payload[1] == 0x00) {
            uint8_t* source_mac = payload + 4;
            ESP_LOGI(TAG, "Pixie Dust attack detected from MAC address: %02X:%02X:%02X:%02X:%02X:%02X on channel %d",
                     source_mac[0], source_mac[1], source_mac[2], source_mac[3], source_mac[4], source_mac[5], rx_ctrl.channel);
            sound_alarm();
        }
    }
}

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    sound_alarm();
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
    ESP_ERROR_CHECK()
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi Sniffer started");
}