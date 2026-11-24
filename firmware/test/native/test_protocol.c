#include "vb_protocol.h"
#include "vb_button.h"
#include <stdio.h>
#include <string.h>

static int g_failures;

#define RUN(name) do { printf("  %s...", #name); name(); printf(" ok\n"); } while (0)
#define ASSERT(cond) do { if (!(cond)) { printf("\nFAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failures++; return; } } while (0)
#define ASSERT_EQ(a, b) ASSERT((a) == (b))

static uint8_t s_err_code;
static uint8_t s_last_evt;
static uint8_t s_last_pl[8];
static uint8_t s_last_len;

static void on_err(uint8_t code, void *ctx)
{
    (void)ctx;
    s_err_code = code;
}

static void on_notify(uint8_t evt, const uint8_t *pl, uint8_t len, void *ctx)
{
    (void)ctx;
    s_last_evt = evt;
    if (len > 0 && pl != NULL) {
        memcpy(s_last_pl, pl, len);
    }
    s_last_len = len;
}

static void test_crc8_vectors(void)
{
    const uint8_t s1[] = { 1, 1, 0 };
    const uint8_t s2[] = { 1, 5, 0 };
    const uint8_t s3[] = { 1, 1, 1, 5 };
    ASSERT_EQ(vb_crc8(s1, sizeof(s1)), 126);
    ASSERT_EQ(vb_crc8(s2, sizeof(s2)), 42);
    ASSERT_EQ(vb_crc8(s3, sizeof(s3)), 115);
}

static void test_encode_request_status(void)
{
    uint8_t out[16];
    size_t n = vb_encode_frame(VB_CMD_REQUEST_STATUS, NULL, 0, out, sizeof(out));
    ASSERT_EQ(n, 6);
    ASSERT_EQ(out[0], 0x42);
    ASSERT_EQ(out[1], 0x56);
    ASSERT_EQ(out[3], VB_CMD_REQUEST_STATUS);
    ASSERT_EQ(out[4], 0);
    ASSERT_EQ(out[5], 0x2a);
}

static void test_encode_set_preset_5(void)
{
    uint8_t pl[] = { 5 };
    uint8_t out[16];
    size_t n = vb_encode_frame(VB_CMD_SET_PRESET, pl, 1, out, sizeof(out));
    ASSERT_EQ(n, 7);
    ASSERT_EQ(out[5], 5);
    ASSERT_EQ(out[6], 0x73);
}

static void test_decode_roundtrip(void)
{
    uint8_t raw[] = { 0x42, 0x56, 0x01, 0x08, 0x00, 0xc3 };
    vb_frame_t frame;
    ASSERT(vb_decode_frame(raw, sizeof(raw), &frame));
    ASSERT_EQ(frame.cmd, VB_CMD_PING);
    ASSERT_EQ(frame.payload_len, 0);
}

static void test_decode_bad_crc(void)
{
    uint8_t raw[] = { 0x42, 0x56, 0x01, 0x05, 0x00, 0x00 };
    vb_frame_t frame;
    ASSERT(!vb_decode_frame(raw, sizeof(raw), &frame));
}

static void test_handle_set_preset_invalid(void)
{
    vb_state_t st = { 0 };
    vb_frame_t frame = { .cmd = VB_CMD_SET_PRESET, .payload = { 99 }, .payload_len = 1 };
    s_err_code = 0;
    ASSERT(!vb_handle_command(&frame, &st, NULL, on_err, NULL));
    ASSERT_EQ(s_err_code, VB_ERR_INVALID_PRESET);
}

static void test_handle_set_live_and_status_notify(void)
{
    vb_state_t st = { .gain = 80, .battery = 100 };
    vb_frame_t frame = { .cmd = VB_CMD_SET_LIVE, .payload = { 1 }, .payload_len = 1 };
    s_last_evt = 0;
    s_last_len = 0;
    ASSERT(vb_handle_command(&frame, &st, on_notify, NULL, NULL));
    ASSERT(st.live);
    ASSERT_EQ(s_last_evt, VB_EVT_STATUS);
    ASSERT_EQ(s_last_len, 6);
    ASSERT_EQ(s_last_pl[1], 1);
}

static void test_factory_reset(void)
{
    vb_state_t st = { .preset_index = 9, .live = true, .muted = true, .gain = 40, .flags = 3 };
    vb_frame_t frame = { .cmd = VB_CMD_FACTORY_RESET, .payload_len = 0 };
    s_last_evt = 0;
    ASSERT(vb_handle_command(&frame, &st, on_notify, NULL, NULL));
    ASSERT_EQ(st.preset_index, 0);
    ASSERT(!st.live);
    ASSERT(!st.muted);
    ASSERT_EQ(st.gain, 80);
    ASSERT_EQ(s_last_evt, VB_EVT_STATUS);
}

static void test_button_short_double_long(void)
{
    vb_button_t btn;
    vb_button_event_t ev;

    vb_button_init(&btn, 30, 350, 800);
    vb_button_poll(&btn, true, 0);
    vb_button_poll(&btn, false, 40);
    vb_button_poll(&btn, true, 400);
    ev = vb_button_poll(&btn, true, 800);
    ASSERT_EQ(ev, VB_BUTTON_SHORT);

    vb_button_init(&btn, 30, 350, 800);
    vb_button_poll(&btn, true, 0);
    vb_button_poll(&btn, false, 40);
    vb_button_poll(&btn, true, 80);
    vb_button_poll(&btn, false, 120);
    vb_button_poll(&btn, true, 160);
    ev = vb_button_poll(&btn, true, 520);
    ASSERT_EQ(ev, VB_BUTTON_DOUBLE);

    vb_button_init(&btn, 30, 350, 800);
    vb_button_poll(&btn, true, 0);
    vb_button_poll(&btn, false, 40);
    ev = vb_button_poll(&btn, false, 900);
    ASSERT_EQ(ev, VB_BUTTON_LONG);
}

int main(void)
{
    g_failures = 0;
    printf("voice-brooch protocol tests\n");
    RUN(test_crc8_vectors);
    RUN(test_encode_request_status);
    RUN(test_encode_set_preset_5);
    RUN(test_decode_roundtrip);
    RUN(test_decode_bad_crc);
    RUN(test_handle_set_preset_invalid);
    RUN(test_handle_set_live_and_status_notify);
    RUN(test_factory_reset);
    RUN(test_button_short_double_long);

    if (g_failures) {
        printf("%d test(s) failed\n", g_failures);
        return 1;
    }
    printf("all passed\n");
    return 0;
}
