#ifndef BLE_GATT_H
#define BLE_GATT_H

#include "vb_app.h"
#include <stddef.h>
#include <stdint.h>

int ble_gatt_init(vb_app_t *app);
void ble_gatt_notify_status(uint8_t evt_cmd, const uint8_t *payload, uint8_t len, void *ctx);
void ble_gatt_notify_event(uint8_t evt_cmd, const uint8_t *payload, uint8_t len, void *ctx);
int ble_gatt_device_info_read(uint8_t *out, size_t cap);

#endif
