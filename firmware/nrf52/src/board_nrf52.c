#include "board_nrf52.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)

static const struct gpio_dt_spec s_button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec s_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec s_charge = GPIO_DT_SPEC_GET(DT_ALIAS(charge0), gpios);
static const struct adc_dt_spec s_mic_adc = ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);
static const struct adc_dt_spec s_batt_adc = ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);
static int16_t s_adc_buf;

#else /* DK fallback pins — apply voice_brooch.overlay for PCB */

#define PIN_BUTTON  11
#define PIN_LED     13
#define PIN_CHARGE  12

static const struct device *s_gpio0;
static const struct device *s_adc_dev;
static struct adc_channel_cfg s_mic_cfg;
static struct adc_channel_cfg s_batt_cfg;
static int16_t s_adc_buf;

#endif

void board_nrf52_init(void)
{
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    if (!gpio_is_ready_dt(&s_button) || !gpio_is_ready_dt(&s_led) ||
        !gpio_is_ready_dt(&s_charge)) {
        return;
    }
    gpio_pin_configure_dt(&s_button, GPIO_INPUT);
    gpio_pin_configure_dt(&s_led, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&s_charge, GPIO_INPUT);
    if (adc_is_ready_dt(&s_mic_adc)) {
        adc_channel_setup_dt(&s_mic_adc);
    }
    if (adc_is_ready_dt(&s_batt_adc)) {
        adc_channel_setup_dt(&s_batt_adc);
    }
#else
    s_gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    s_adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    gpio_pin_configure(s_gpio0, PIN_LED, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_HIGH);
    gpio_pin_configure(s_gpio0, PIN_BUTTON, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure(s_gpio0, PIN_CHARGE, GPIO_INPUT | GPIO_PULL_UP);
    s_mic_cfg = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_6,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id = 1,
        .input_positive = SAADC_CH_PSELP_PSELP_AnalogInput1
    };
    s_batt_cfg = s_mic_cfg;
    s_batt_cfg.channel_id = 2;
    s_batt_cfg.input_positive = SAADC_CH_PSELP_PSELP_AnalogInput2;
    adc_channel_setup(s_adc_dev, &s_mic_cfg);
    adc_channel_setup(s_adc_dev, &s_batt_cfg);
#endif
}

#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)

static uint16_t read_adc_dt(const struct adc_dt_spec *spec)
{
    struct adc_sequence seq = ADC_DT_SPEC_GET_SEQ(*spec, &s_adc_buf, sizeof(s_adc_buf));
    if (!adc_is_ready_dt(spec) || adc_read_dt(spec, &seq) != 0) {
        return 0;
    }
    return (uint16_t)(s_adc_buf < 0 ? 0 : s_adc_buf);
}

#else

static uint16_t read_adc_channel(uint8_t ch_id)
{
    struct adc_sequence seq = {
        .channels = BIT(ch_id),
        .buffer = &s_adc_buf,
        .buffer_size = sizeof(s_adc_buf),
        .resolution = 12
    };
    if (adc_read(s_adc_dev, &seq) != 0) {
        return 0;
    }
    return (uint16_t)(s_adc_buf < 0 ? 0 : s_adc_buf);
}

#endif

static uint16_t read_mic(void *ctx)
{
    ARG_UNUSED(ctx);
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    return read_adc_dt(&s_mic_adc);
#else
    return read_adc_channel(1);
#endif
}

static uint16_t read_batt(void *ctx)
{
    ARG_UNUSED(ctx);
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    return (uint16_t)(read_adc_dt(&s_batt_adc) * 4200 / 4095);
#else
    return (uint16_t)(read_adc_channel(2) * 4200 / 4095);
#endif
}

static bool read_button(void *ctx)
{
    ARG_UNUSED(ctx);
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    return gpio_pin_get_dt(&s_button) != 0;
#else
    return gpio_pin_get(s_gpio0, PIN_BUTTON) != 0;
#endif
}

static bool is_charging(void *ctx)
{
    ARG_UNUSED(ctx);
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    return gpio_pin_get_dt(&s_charge) != 0;
#else
    return gpio_pin_get(s_gpio0, PIN_CHARGE) != 0;
#endif
}

static uint32_t millis(void *ctx)
{
    ARG_UNUSED(ctx);
    return (uint32_t)k_uptime_get();
}

static void set_led(bool on, void *ctx)
{
    ARG_UNUSED(ctx);
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
    gpio_pin_set_dt(&s_led, on ? 1 : 0);
#else
    gpio_pin_set(s_gpio0, PIN_LED, on ? 1 : 0);
#endif
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
