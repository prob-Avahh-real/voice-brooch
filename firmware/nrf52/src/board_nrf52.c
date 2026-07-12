#include "board_nrf52.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>

#define PIN_BUTTON  11
#define PIN_LED     13
#define PIN_CHARGE  12

static const struct device *gpio0;
static const struct device *adc_dev;
static struct adc_channel_cfg mic_cfg;
static struct adc_channel_cfg batt_cfg;
static int16_t adc_buf;

void board_nrf52_init(void)
{
    gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));

    gpio_pin_configure(gpio0, PIN_LED, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_HIGH);
    gpio_pin_configure(gpio0, PIN_BUTTON, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(gpio0, PIN_CHARGE, GPIO_INPUT | GPIO_PULL_UP);

    mic_cfg = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_6,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id = 1,
        .input_positive = SAADC_CH_PSELP_PSELP_AnalogInput1
    };
    batt_cfg = mic_cfg;
    batt_cfg.channel_id = 2;
    batt_cfg.input_positive = SAADC_CH_PSELP_PSELP_AnalogInput2;

    adc_channel_setup(adc_dev, &mic_cfg);
    adc_channel_setup(adc_dev, &batt_cfg);
}

static uint16_t read_adc_channel(uint8_t ch_id)
{
    struct adc_sequence seq = {
        .channels = BIT(ch_id),
        .buffer = &adc_buf,
        .buffer_size = sizeof(adc_buf),
        .resolution = 12
    };
    if (adc_read(adc_dev, &seq) != 0) {
        return 0;
    }
    return (uint16_t)(adc_buf < 0 ? 0 : adc_buf);
}

static uint16_t read_mic(void *ctx)
{
    ARG_UNUSED(ctx);
    return read_adc_channel(1);
}

static uint16_t read_batt(void *ctx)
{
    ARG_UNUSED(ctx);
    return (uint16_t)(read_adc_channel(2) * 4200 / 4095);
}

static bool read_button(void *ctx)
{
    ARG_UNUSED(ctx);
    return gpio_pin_get(gpio0, PIN_BUTTON) != 0;
}

static bool is_charging(void *ctx)
{
    ARG_UNUSED(ctx);
    return gpio_pin_get(gpio0, PIN_CHARGE) != 0;
}

static uint32_t millis(void *ctx)
{
    ARG_UNUSED(ctx);
    return (uint32_t)k_uptime_get();
}

static void set_led(bool on, void *ctx)
{
    ARG_UNUSED(ctx);
    gpio_pin_set(gpio0, PIN_LED, on ? 1 : 0);
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

const vb_board_ops_t *board_nrf52_ops(void)
{
    return &s_ops;
}
