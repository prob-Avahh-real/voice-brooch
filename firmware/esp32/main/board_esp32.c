#include "board_esp32.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PIN_BUTTON   GPIO_NUM_0
#define PIN_LED      GPIO_NUM_2
#define PIN_MIC_ADC  ADC_CHANNEL_6   /* GPIO34 */
#define PIN_BATT_ADC ADC_CHANNEL_7   /* GPIO35 */
#define PIN_CHARGE   GPIO_NUM_36

static adc_oneshot_unit_handle_t s_adc;

void board_esp32_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_BUTTON) | (1ULL << PIN_CHARGE),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);

    io.pin_bit_mask = (1ULL << PIN_LED);
    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io);
    gpio_set_level(PIN_LED, 0);

    adc_oneshot_unit_init_cfg_t init = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init, &s_adc);
    adc_oneshot_chan_cfg_t ch = { .bitwidth = ADC_BITWIDTH_12, .atten = ADC_ATTEN_DB_12 };
    adc_oneshot_config_channel(s_adc, PIN_MIC_ADC, &ch);
    adc_oneshot_config_channel(s_adc, PIN_BATT_ADC, &ch);
}

static uint16_t read_adc(adc_channel_t ch)
{
    int raw = 0;
    adc_oneshot_read(s_adc, ch, &raw);
    return (uint16_t)(raw < 0 ? 0 : raw);
}

static uint16_t read_mic(void *ctx)
{
    (void)ctx;
    return read_adc(PIN_MIC_ADC);
}

static uint16_t read_batt(void *ctx)
{
    (void)ctx;
    /* 分压后映射到 2800–4200 mV 区间，此处直接读 ADC 演示 */
    return (uint16_t)(read_adc(PIN_BATT_ADC) * 4200 / 4095);
}

static bool read_button(void *ctx)
{
    (void)ctx;
    return gpio_get_level(PIN_BUTTON) == 1;
}

static bool is_charging(void *ctx)
{
    (void)ctx;
    return gpio_get_level(PIN_CHARGE) == 1;
}

static uint32_t millis(void *ctx)
{
    (void)ctx;
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static void set_led(bool on, void *ctx)
{
    (void)ctx;
    gpio_set_level(PIN_LED, on ? 1 : 0);
}

static const vb_board_ops_t s_ops = {
    .read_mic_adc = read_mic,
    .read_batt_adc = read_batt,
    .read_button = read_button,
    .is_charging = is_charging,
    .millis = millis,
    .set_led = set_led,
    .board_ctx = NULL
};

const vb_board_ops_t *board_esp32_ops(void)
{
    return &s_ops;
}
