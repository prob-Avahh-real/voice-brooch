/**
 * 变声胸针 BLE 协议 v1 — 固件共享层
 * 与 src/hardware/ble/protocol.js 保持一致
 */
#ifndef VB_PROTOCOL_H
#define VB_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VB_PROTO_VER           1
#define VB_SOF0                  0x42
#define VB_SOF1                  0x56
#define VB_PRESET_COUNT          18
#define VB_MAX_PAYLOAD           32

/* App → Brooch */
#define VB_CMD_SET_PRESET        0x01
#define VB_CMD_SET_LIVE          0x02
#define VB_CMD_SET_MUTE          0x03
#define VB_CMD_SYNC_TIME         0x04
#define VB_CMD_REQUEST_STATUS    0x05
#define VB_CMD_SET_GAIN          0x06
#define VB_CMD_FACTORY_RESET     0x07
#define VB_CMD_PING              0x08

/* Brooch → App */
#define VB_EVT_STATUS            0x80
#define VB_EVT_BUTTON            0x81
#define VB_EVT_PRESET_CHANGED    0x82
#define VB_EVT_AUDIO_LEVEL       0x83
#define VB_EVT_LOW_BATTERY       0x84
#define VB_EVT_PONG              0x85
#define VB_EVT_ERROR             0x8F

#define VB_BTN_SHORT             0x01
#define VB_BTN_DOUBLE            0x02
#define VB_BTN_LONG              0x03
#define VB_BTN_ID_MAIN           0x01

#define VB_ERR_INVALID_PRESET    0x01
#define VB_ERR_MIC_NOT_READY     0x02
#define VB_ERR_CRC_FAIL          0x03
#define VB_ERR_UNSUPPORTED_CMD   0x04

#define VB_FLAG_CHARGING         (1 << 0)
#define VB_FLAG_MIC_FAULT        (1 << 1)

#define VB_UUID128_SERVICE       0xA1B2C001, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB
#define VB_UUID128_CONTROL       0xA1B2C002, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB
#define VB_UUID128_STATUS        0xA1B2C003, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB
#define VB_UUID128_EVENT         0xA1B2C004, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB
#define VB_UUID128_DEVICE_INFO   0xA1B2C005, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB

typedef struct {
    uint8_t preset_index;
    bool live;
    bool muted;
    uint8_t battery;
    uint8_t gain;
    uint8_t flags;
} vb_state_t;

typedef struct {
    uint8_t cmd;
    uint8_t payload[VB_MAX_PAYLOAD];
    uint8_t payload_len;
} vb_frame_t;

typedef void (*vb_notify_fn)(uint8_t evt_cmd, const uint8_t *payload, uint8_t len, void *ctx);
typedef void (*vb_error_fn)(uint8_t code, void *ctx);

uint8_t vb_crc8(const uint8_t *data, size_t len);
size_t vb_encode_frame(uint8_t cmd, const uint8_t *payload, uint8_t payload_len, uint8_t *out, size_t out_cap);
bool vb_decode_frame(const uint8_t *data, size_t len, vb_frame_t *out);

size_t vb_build_status_payload(const vb_state_t *st, uint8_t *out, size_t cap);
size_t vb_build_button_payload(uint8_t button_id, uint8_t action, uint8_t *out, size_t cap);
size_t vb_build_preset_changed_payload(uint8_t index, uint8_t source, uint8_t *out, size_t cap);

bool vb_handle_command(const vb_frame_t *frame, vb_state_t *st,
                       vb_notify_fn notify, vb_error_fn on_error, void *ctx);

#endif /* VB_PROTOCOL_H */
