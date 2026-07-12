#ifndef VB_APP_H
#define VB_APP_H

#include "vb_protocol.h"
#include "vb_button.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct vb_app vb_app_t;

typedef struct {
    uint16_t (*read_mic_adc)(void *ctx);
    uint16_t (*read_batt_adc)(void *ctx);
    bool (*read_button)(void *ctx);
    bool (*is_charging)(void *ctx);
    uint32_t (*millis)(void *ctx);
    void (*set_led)(bool on, void *ctx);
    void *board_ctx;
} vb_board_ops_t;

typedef struct {
    vb_notify_fn ble_notify_status;
    vb_notify_fn ble_notify_event;
    void *ble_ctx;
} vb_ble_ops_t;

struct vb_app {
    vb_state_t state;
    vb_button_t button;
    vb_board_ops_t board;
    vb_ble_ops_t ble;
    uint32_t last_level_ms;
    uint32_t last_batt_ms;
    bool connected;
};

void vb_app_init(vb_app_t *app, const vb_board_ops_t *board, const vb_ble_ops_t *ble);
void vb_app_set_connected(vb_app_t *app, bool connected);
void vb_app_on_control_write(vb_app_t *app, const uint8_t *data, size_t len);
void vb_app_tick(vb_app_t *app);
const vb_state_t *vb_app_state(const vb_app_t *app);

#endif /* VB_APP_H */
