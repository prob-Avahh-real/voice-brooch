#include "board_nrf52.h"
#include "ble_gatt.h"
#include "vb_app.h"

#include <zephyr/kernel.h>

static vb_app_t s_app;

int main(void)
{
    board_nrf52_init();

    vb_ble_ops_t ble_ops = {
        .ble_notify_status = ble_gatt_notify_status,
        .ble_notify_event = ble_gatt_notify_event,
        .ble_ctx = NULL
    };
    vb_app_init(&s_app, board_nrf52_ops(), &ble_ops);

    if (ble_gatt_init(&s_app) != 0) {
        return -1;
    }

    while (1) {
        vb_app_tick(&s_app);
        k_sleep(K_MSEC(10));
    }

    return 0;
}
