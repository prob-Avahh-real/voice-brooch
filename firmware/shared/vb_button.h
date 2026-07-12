#ifndef VB_BUTTON_H
#define VB_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    VB_BUTTON_NONE = 0,
    VB_BUTTON_SHORT,
    VB_BUTTON_DOUBLE,
    VB_BUTTON_LONG
} vb_button_event_t;

typedef struct {
    uint32_t debounce_ms;
    uint32_t double_ms;
    uint32_t long_ms;
    bool pressed;
    uint32_t press_ts;
    uint32_t last_release_ts;
    uint8_t click_count;
    bool long_fired;
} vb_button_t;

void vb_button_init(vb_button_t *btn, uint32_t debounce_ms, uint32_t double_ms, uint32_t long_ms);
vb_button_event_t vb_button_poll(vb_button_t *btn, bool raw_level, uint32_t now_ms);

#endif /* VB_BUTTON_H */
