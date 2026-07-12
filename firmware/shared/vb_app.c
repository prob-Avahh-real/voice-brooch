#include "vb_app.h"
#include <string.h>

static void app_notify_bridge(uint8_t evt_cmd, const uint8_t *payload, uint8_t len, void *ctx)
{
    vb_app_t *app = (vb_app_t *)ctx;
    if (evt_cmd == VB_EVT_STATUS && app->ble.ble_notify_status) {
        app->ble.ble_notify_status(evt_cmd, payload, len, app->ble.ble_ctx);
    } else if (app->ble.ble_notify_event) {
        app->ble.ble_notify_event(evt_cmd, payload, len, app->ble.ble_ctx);
    }
}

static void app_on_error(uint8_t code, void *ctx)
{
    vb_app_t *app = (vb_app_t *)ctx;
    if (app->ble.ble_notify_event) {
        app->ble.ble_notify_event(VB_EVT_ERROR, &code, 1, app->ble.ble_ctx);
    }
}

static void send_status(vb_app_t *app)
{
    uint8_t pl[6];
    size_t n = vb_build_status_payload(&app->state, pl, sizeof(pl));
    if (n && app->ble.ble_notify_status) {
        app->ble.ble_notify_status(VB_EVT_STATUS, pl, (uint8_t)n, app->ble.ble_ctx);
    }
}

static void send_button(vb_app_t *app, uint8_t action)
{
    uint8_t pl[2];
    size_t n = vb_build_button_payload(VB_BTN_ID_MAIN, action, pl, sizeof(pl));
    if (n && app->ble.ble_notify_event) {
        app->ble.ble_notify_event(VB_EVT_BUTTON, pl, (uint8_t)n, app->ble.ble_ctx);
    }
}

static void handle_button_event(vb_app_t *app, vb_button_event_t ev)
{
    uint8_t pl[2];
    size_t n;

    switch (ev) {
    case VB_BUTTON_SHORT:
        app->state.live = !app->state.live;
        send_button(app, VB_BTN_SHORT);
        send_status(app);
        if (app->board.set_led) {
            app->board.set_led(app->state.live && !app->state.muted, app->board.board_ctx);
        }
        break;

    case VB_BUTTON_DOUBLE:
        app->state.preset_index = (uint8_t)((app->state.preset_index + 1) % VB_PRESET_COUNT);
        send_button(app, VB_BTN_DOUBLE);
        n = vb_build_preset_changed_payload(app->state.preset_index, 1, pl, sizeof(pl));
        if (n && app->ble.ble_notify_event) {
            app->ble.ble_notify_event(VB_EVT_PRESET_CHANGED, pl, (uint8_t)n, app->ble.ble_ctx);
        }
        send_status(app);
        break;

    case VB_BUTTON_LONG:
        app->state.muted = !app->state.muted;
        send_button(app, VB_BTN_LONG);
        send_status(app);
        if (app->board.set_led) {
            app->board.set_led(app->state.live && !app->state.muted, app->board.board_ctx);
        }
        break;

    default:
        break;
    }
}

void vb_app_init(vb_app_t *app, const vb_board_ops_t *board, const vb_ble_ops_t *ble)
{
    memset(app, 0, sizeof(*app));
    app->state.preset_index = 0;
    app->state.gain = 80;
    app->state.battery = 100;
    app->board = *board;
    app->ble = *ble;
    vb_button_init(&app->button, 30, 350, 800);
}

void vb_app_set_connected(vb_app_t *app, bool connected)
{
    app->connected = connected;
    if (connected) {
        send_status(app);
    }
}

void vb_app_on_control_write(vb_app_t *app, const uint8_t *data, size_t len)
{
    vb_frame_t frame;
    if (!vb_decode_frame(data, len, &frame)) {
        app_on_error(VB_ERR_CRC_FAIL, app);
        return;
    }
    vb_handle_command(&frame, &app->state, app_notify_bridge, app_on_error, app);
    if (app->board.set_led) {
        app->board.set_led(app->state.live && !app->state.muted, app->board.board_ctx);
    }
}

static uint8_t adc_to_level(uint16_t adc, uint16_t max_adc)
{
    if (max_adc == 0) return 0;
    uint32_t v = (uint32_t)adc * 255u / max_adc;
    return v > 255 ? 255 : (uint8_t)v;
}

static uint8_t adc_to_battery(uint16_t adc, uint16_t min_adc, uint16_t max_adc)
{
    if (adc <= min_adc) return 0;
    if (adc >= max_adc) return 100;
    uint32_t v = (uint32_t)(adc - min_adc) * 100u / (max_adc - min_adc);
    return v > 100 ? 100 : (uint8_t)v;
}

void vb_app_tick(vb_app_t *app)
{
    uint32_t now = app->board.millis(app->board.board_ctx);

    if (app->board.read_button) {
        vb_button_event_t ev = vb_button_poll(&app->button,
                                                app->board.read_button(app->board.board_ctx),
                                                now);
        if (ev != VB_BUTTON_NONE) {
            handle_button_event(app, ev);
        }
    }

    if (app->connected && app->board.read_mic_adc &&
        (now - app->last_level_ms) >= 50) {
        app->last_level_ms = now;
        uint16_t mic = app->board.read_mic_adc(app->board.board_ctx);
        uint8_t level = adc_to_level(mic, 4095);
        if (app->ble.ble_notify_event) {
            app->ble.ble_notify_event(VB_EVT_AUDIO_LEVEL, &level, 1, app->ble.ble_ctx);
        }
    }

    if (app->board.read_batt_adc && (now - app->last_batt_ms) >= 5000) {
        app->last_batt_ms = now;
        uint16_t batt_adc = app->board.read_batt_adc(app->board.board_ctx);
        app->state.battery = adc_to_battery(batt_adc, 2800, 4200);
        app->state.flags = 0;
        if (app->board.is_charging && app->board.is_charging(app->board.board_ctx)) {
            app->state.flags |= VB_FLAG_CHARGING;
        }
        if (app->state.battery <= 15 && app->connected && app->ble.ble_notify_event) {
            uint8_t b = app->state.battery;
            app->ble.ble_notify_event(VB_EVT_LOW_BATTERY, &b, 1, app->ble.ble_ctx);
        }
    }
}

const vb_state_t *vb_app_state(const vb_app_t *app)
{
    return &app->state;
}
