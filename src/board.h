#ifndef BOARD_H_
#define BOARD_H_
#include "constants.h"
#include "pin_modes.h"

#define STRLEN_PIN_NR 3
#define STRLEN_MODE_NR 3
#define STRLEN_JSON_ALLOWED (STRLEN_PIN_NR + 1) * NUM_PINS + 3
#define STRLEN_JSON_MODE strlen(TEMPLATE_JSON_MODE) + STRLEN_JSON_ALLOWED + STRLEN_MODE_NR + STRLEN_MODE_NAME + 1
#define STRLEN_JSON_MODES (STRLEN_JSON_MODE + 1) * NUM_MODES + 3

void board_init(void);

void board_to_html(char *content);

void board_modes_as_json(char *json);

void board_init_pin_modes(const pin_mode_nr_t new_modes[NUM_PINS]);

err_t board_pin_operation(pin_nr_t pin_nr, double *val, pin_dir_t dir);

#endif
