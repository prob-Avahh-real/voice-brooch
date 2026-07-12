#ifndef BOARD_ESP32_H
#define BOARD_ESP32_H

#include "vb_app.h"

void board_esp32_init(void);
const vb_board_ops_t *board_esp32_ops(void);

#endif
