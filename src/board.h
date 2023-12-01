#ifndef BOARD_H_
#define BOARD_H_
#include "constants.h"
#include "pin_modes.h"

#define LEN_JS_PIN_STAT 3
#define LEN_JS_PIN_STATS (LEN_JS_PIN_STAT + 1) * 3 + 4
#define LEN_JS_PINS (LEN_JS_PIN_STATS + 1) * NUM_PINS + 4

#define LEN_JS_NAMES (LEN_MODE_NAME + 5) * NUM_MODES

#define LEN_HTML_TEMPLATE strlen(HTML_TEMPLATE) + LEN_JS_PINS + LEN_JS_NAMES

void board_init(void);

void board_to_html(char *content);

void board_init_pin_modes(const pin_mode_nr_t new_modes[NUM_PINS]);

err_t board_pin_operation(pin_nr_t pin_nr, double *val);

#endif
