#include "vb_button.h"

void vb_button_init(vb_button_t *btn, uint32_t debounce_ms, uint32_t double_ms, uint32_t long_ms)
{
    btn->debounce_ms = debounce_ms;
    btn->double_ms = double_ms;
    btn->long_ms = long_ms;
    btn->pressed = false;
    btn->press_ts = 0;
    btn->last_release_ts = 0;
    btn->click_count = 0;
    btn->long_fired = false;
}

vb_button_event_t vb_button_poll(vb_button_t *btn, bool raw_level, uint32_t now_ms)
{
    /* active-low button: raw_level=false means pressed */
    bool is_pressed = !raw_level;

    if (is_pressed && !btn->pressed) {
        if ((now_ms - btn->last_release_ts) >= btn->debounce_ms) {
            btn->pressed = true;
            btn->press_ts = now_ms;
            btn->long_fired = false;
        }
    } else if (!is_pressed && btn->pressed) {
        btn->pressed = false;
        btn->last_release_ts = now_ms;
        if (!btn->long_fired) {
            btn->click_count++;
        }
    }

    if (btn->pressed && !btn->long_fired && (now_ms - btn->press_ts) >= btn->long_ms) {
        btn->long_fired = true;
        btn->click_count = 0;
        return VB_BUTTON_LONG;
    }

    if (!btn->pressed && btn->click_count > 0 &&
        (now_ms - btn->last_release_ts) >= btn->double_ms) {
        vb_button_event_t ev = (btn->click_count >= 2) ? VB_BUTTON_DOUBLE : VB_BUTTON_SHORT;
        btn->click_count = 0;
        return ev;
    }

    return VB_BUTTON_NONE;
}
