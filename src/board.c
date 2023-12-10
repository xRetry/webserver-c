#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "board.h"
#include "constants.h"
#include "utils.h"

#include "pin_modes.h"


struct board_t {
    struct state_t {
        pin_mode_nr_t mode_nrs[NUM_PINS];
    } state;

    err_t (*rw_functions[NUM_PINS])(pin_nr_t, double*);

    pin_dir_t directions[NUM_PINS];

    pin_mode_nr_t allowed_modes[NUM_PINS];
} board;

void allowed_init(void) {
    for (int i=0; i<NUM_PINS; ++i) {
        board.allowed_modes[i] = 0;
    }

    for (int i=0; i<NUM_MODES; ++i) {
        struct pin_mode_t pin_mode = PIN_MODES[i];
        for (int j=0; j<pin_mode.pins_allowed_size; ++j) {
            board.allowed_modes[j] |= 1<<i;
        }
    }
}

void state_init(void) {
    err_t err = utils_read_binary("modes", board.state.mode_nrs, sizeof(pin_mode_nr_t), NUM_PINS);
    if (err != 0) {
        for (int i=0; i<NUM_PINS; ++i) {
            board.state.mode_nrs[i] = 0;
        }
    }
}

void modes_init(void) {
    for (int i=0; i<NUM_PINS; ++i) {
        struct pin_mode_t pin_mode = PIN_MODES[board.state.mode_nrs[i]];
        pin_mode.fn_init(i);
        board.rw_functions[i] = pin_mode.fn_rw;
        board.directions[i] = pin_mode.direction;
    }
}

void board_init(void) {
    allowed_init();
    state_init();
    modes_init();
}

err_t board_pin_operation(pin_nr_t pin_nr, double *val, pin_dir_t dir) {
    if (pin_nr >= NUM_PINS || board.directions[pin_nr] != dir) { 
        return 1; 
    }

    board.rw_functions[pin_nr](pin_nr, val);
    return 0;
}

void board_init_pin_modes(const pin_mode_nr_t new_modes_nrs[NUM_PINS]) {
    for (int i=0; i<NUM_PINS; ++i) {
        struct pin_mode_t pin_mode = PIN_MODES[new_modes_nrs[i]];

        bool is_mode_allowed = false;
        for (int j=0; j<pin_mode.pins_allowed_size; ++j) {
            if (pin_mode.pins_allowed[j] == i) {
                is_mode_allowed = true;
                break;
            }
        }

        if (!is_mode_allowed || ERR(pin_mode.fn_init(i))) { 
            // TODO(marco): Save errors
            continue; 
        };
        board.rw_functions[i] = pin_mode.fn_rw;
        board.directions[i] = pin_mode.direction;
        board.state.mode_nrs[i] = new_modes_nrs[i];
    }
    utils_write_binary("modes", board.state.mode_nrs, sizeof(pin_mode_nr_t), NUM_PINS);
}

void board_modes_as_json(char *json) {
    json[0] = '\0';

    strcat(json, "[");
    for (int i=0; i<NUM_MODES; ++i) {
        char *comma = i == 0 ? "\0" : ",";
        strcat(json, comma);

        char json_allowed[STRLEN_JSON_ALLOWED] = "[";
        for (int j=0; j<PIN_MODES[i].pins_allowed_size; ++j) {
            char *c = j == 0 ? "\0" : ",";
            strcat(json_allowed, c);
            char pin_nr[STRLEN_PIN_NR];
            snprintf(pin_nr, STRLEN_PIN_NR, "%d", PIN_MODES[i].pins_allowed[j]);
            strcat(json_allowed, pin_nr);
        }
        strcat(json_allowed, "]");

        char json_mode[STRLEN_JSON_MODES];
        snprintf(json_mode, STRLEN_JSON_MODES, TEMPLATE_JSON_MODE, i, PIN_MODES[i].name, json_allowed);
        strcat(json, json_mode);
    }
    strcat(json, "]");
}

void board_active_as_json(char *json) {
    json[0] = '\0';

    strcat(json, "{");
    for (int i=0; i<NUM_PINS; ++i) {
        char *comma = i == 0 ? "\0" : ",";
        strcat(json, comma);

        char json_mode[STRLEN_JSON_ACTIVE_ENTRY];
        snprintf(json_mode, STRLEN_JSON_ACTIVE_ENTRY, "\"%d\":%d", i, board.state.mode_nrs[i]);
        strcat(json, json_mode);
    }
    strcat(json, "}");
}
