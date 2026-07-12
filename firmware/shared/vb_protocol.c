#include "vb_protocol.h"
#include <string.h>

uint8_t vb_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

size_t vb_encode_frame(uint8_t cmd, const uint8_t *payload, uint8_t payload_len,
                       uint8_t *out, size_t out_cap)
{
    if (payload_len > VB_MAX_PAYLOAD || out_cap < (size_t)(6 + payload_len)) {
        return 0;
    }

    uint8_t body[3 + VB_MAX_PAYLOAD];
    body[0] = VB_PROTO_VER;
    body[1] = cmd;
    body[2] = payload_len;
    if (payload_len > 0 && payload != NULL) {
        memcpy(&body[3], payload, payload_len);
    }

    out[0] = VB_SOF0;
    out[1] = VB_SOF1;
    memcpy(&out[2], body, 3 + payload_len);
    out[5 + payload_len] = vb_crc8(body, 3 + payload_len);
    return 6 + payload_len;
}

bool vb_decode_frame(const uint8_t *data, size_t len, vb_frame_t *out)
{
    if (len < 6 || data[0] != VB_SOF0 || data[1] != VB_SOF1) {
        return false;
    }
    if (data[2] != VB_PROTO_VER) {
        return false;
    }

    uint8_t cmd = data[3];
    uint8_t plen = data[4];
    if (len < (size_t)(5 + plen + 1) || plen > VB_MAX_PAYLOAD) {
        return false;
    }

    uint8_t crc = vb_crc8(&data[2], 3 + plen);
    if (crc != data[5 + plen]) {
        return false;
    }

    out->cmd = cmd;
    out->payload_len = plen;
    if (plen > 0) {
        memcpy(out->payload, &data[5], plen);
    }
    return true;
}

size_t vb_build_status_payload(const vb_state_t *st, uint8_t *out, size_t cap)
{
    if (cap < 6 || st == NULL) return 0;
    out[0] = st->preset_index;
    out[1] = st->live ? 1 : 0;
    out[2] = st->muted ? 1 : 0;
    out[3] = st->battery;
    out[4] = st->gain;
    out[5] = st->flags;
    return 6;
}

size_t vb_build_button_payload(uint8_t button_id, uint8_t action, uint8_t *out, size_t cap)
{
    if (cap < 2) return 0;
    out[0] = button_id;
    out[1] = action;
    return 2;
}

size_t vb_build_preset_changed_payload(uint8_t index, uint8_t source, uint8_t *out, size_t cap)
{
    if (cap < 2) return 0;
    out[0] = index;
    out[1] = source;
    return 2;
}

static void emit_notify(vb_notify_fn notify, uint8_t evt, const uint8_t *payload,
                        uint8_t len, void *ctx)
{
    if (notify != NULL) {
        notify(evt, payload, len, ctx);
    }
}

bool vb_handle_command(const vb_frame_t *frame, vb_state_t *st,
                       vb_notify_fn notify, vb_error_fn on_error, void *ctx)
{
    uint8_t pl[VB_MAX_PAYLOAD];
    size_t plen;

    switch (frame->cmd) {
    case VB_CMD_SET_PRESET:
        if (frame->payload_len < 1) return false;
        if (frame->payload[0] >= VB_PRESET_COUNT) {
            if (on_error) on_error(VB_ERR_INVALID_PRESET, ctx);
            return false;
        }
        st->preset_index = frame->payload[0];
        plen = vb_build_preset_changed_payload(st->preset_index, 0, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_PRESET_CHANGED, pl, (uint8_t)plen, ctx);
        plen = vb_build_status_payload(st, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_STATUS, pl, (uint8_t)plen, ctx);
        break;

    case VB_CMD_SET_LIVE:
        if (frame->payload_len < 1) return false;
        st->live = frame->payload[0] != 0;
        plen = vb_build_status_payload(st, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_STATUS, pl, (uint8_t)plen, ctx);
        break;

    case VB_CMD_SET_MUTE:
        if (frame->payload_len < 1) return false;
        st->muted = frame->payload[0] != 0;
        plen = vb_build_status_payload(st, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_STATUS, pl, (uint8_t)plen, ctx);
        break;

    case VB_CMD_SYNC_TIME:
        /* 可选：存入 RTC，参考实现忽略 */
        break;

    case VB_CMD_REQUEST_STATUS:
        plen = vb_build_status_payload(st, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_STATUS, pl, (uint8_t)plen, ctx);
        break;

    case VB_CMD_SET_GAIN:
        if (frame->payload_len < 1) return false;
        st->gain = frame->payload[0] > 100 ? 100 : frame->payload[0];
        plen = vb_build_status_payload(st, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_STATUS, pl, (uint8_t)plen, ctx);
        break;

    case VB_CMD_FACTORY_RESET:
        st->preset_index = 0;
        st->live = false;
        st->muted = false;
        st->gain = 80;
        st->flags = 0;
        plen = vb_build_status_payload(st, pl, sizeof(pl));
        emit_notify(notify, VB_EVT_STATUS, pl, (uint8_t)plen, ctx);
        break;

    case VB_CMD_PING:
        emit_notify(notify, VB_EVT_PONG, NULL, 0, ctx);
        break;

    default:
        if (on_error) on_error(VB_ERR_UNSUPPORTED_CMD, ctx);
        return false;
    }

    return true;
}
