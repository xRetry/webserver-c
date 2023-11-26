#ifndef BOARD_H_
#define BOARD_H_
#include "constants.h"

void board_init(void);

void board_to_html(char *content);

void board_set_pin_modes(const pin_mode_t new_modes[NUM_PINS]);

#endif
