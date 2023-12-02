#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
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
        if (ERR(pin_mode.fn_init(i))) { 
            // TODO(marco): Save errors
            continue; 
        };
        board.rw_functions[i] = pin_mode.fn_rw;
        board.directions[i] = pin_mode.direction;
        board.state.mode_nrs[i] = new_modes_nrs[i];
    }
    utils_write_binary("modes", board.state.mode_nrs, sizeof(pin_mode_nr_t), NUM_PINS);
}

void board_to_html(char content[LEN_HTML_TEMPLATE]) {
    char pins[LEN_JS_PINS] = "";
    for (int i=0; i<NUM_PINS; ++i) {
        if (board.allowed_modes[i] == 0) { continue; }

        char pin[LEN_JS_PIN_STAT];
        snprintf(pin, LEN_JS_PIN_STAT, "%d", i);
        char allowed[LEN_JS_PIN_STAT];
        snprintf(allowed, LEN_JS_PIN_STAT, "%d", board.allowed_modes[i]);
        char mode[LEN_JS_PIN_STAT];
        snprintf(mode, LEN_JS_PIN_STAT, "%d", board.state.mode_nrs[i]);

        char str[LEN_JS_PIN_STATS] = "";
        char *modes[] = {pin, allowed, mode};

        utils_array_to_json(str, modes, 3, 3);

        strcat(pins, str);
        strcat(pins, ",");
    }

    char *names; 
    if ((names = malloc((LEN_JS_NAMES+1) * sizeof(char))) == NULL) {
        printf("Memory allocation failed!");
        exit(0);
    }

    names[0] = '\0';
    for (int i=0; i<NUM_MODES; ++i) {
        strcat(names, "\"");
        strcat(names, PIN_MODES[i].name);
        strcat(names, "\"");
        strcat(names, ",");
    }

    snprintf(content, LEN_HTML_TEMPLATE, HTML_TEMPLATE, pins, names);
    free(names);
}
