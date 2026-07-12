#include "board_esp32.h"
#include "ble_gatt.h"
#include "vb_app.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "main";
static vb_app_t s_app;

static void app_task(void *arg)
{
    (void)arg;
    while (1) {
        vb_app_tick(&s_app);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    board_esp32_init();

    vb_ble_ops_t ble_ops = {
        .ble_notify_status = ble_gatt_notify_status,
        .ble_notify_event = ble_gatt_notify_event,
        .ble_ctx = NULL
    };
    vb_app_init(&s_app, board_esp32_ops(), &ble_ops);
    ESP_ERROR_CHECK(ble_gatt_init(&s_app) == 0 ? ESP_OK : ESP_FAIL);

    xTaskCreate(app_task, "vb_app", 3072, NULL, 5, NULL);
    ESP_LOGI(TAG, "VoiceBrooch ESP32 firmware ready");
}
