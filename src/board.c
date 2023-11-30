#include <stdio.h>
#include <string.h>
#include "board.h"
#include "utils.h"

#include "pin_modes.h"

struct board_t {
    struct state_t {
        pin_mode_nr_t modes[NUM_PINS];
    } state;
    err_t (*pin_functions[NUM_PINS])(pin_nr_t, double*);

    pin_mode_nr_t allowed_modes[NUM_PINS];

} board;

void allowed_init(void) {
    for (int i=0; i<NUM_PINS; ++i) {
        board.allowed_modes[i] = 0;
    }

    for (int i=0; i<NUM_MODES; ++i) {
        struct pin_mode_t pin_mode = pin_modes[i];
        for (int j=0; j<pin_mode.pins_allowed_size; ++j) {
            board.allowed_modes[j] |= 1<<i;
            board.pin_functions[j] = pin_mode.fn_rw;
        }
    }
}

void state_init(void) {
    err_t err = utils_read_binary("modes", board.state.modes, sizeof(pin_mode_nr_t), NUM_PINS);
    if (err != 0) {
        for (int i=0; i<NUM_PINS; ++i) {
            board.state.modes[i] = 0;
        }
    }
}

void board_init(void) {
    allowed_init();
    state_init();
}

err_t board_pin_operation(pin_nr_t pin_nr, double *val) {
    if (pin_nr >= NUM_PINS) {
        return 1;
    }

    // TODO(marco): Implement proper function routing
    *val = pin_nr;

    printf("pin op: %d - %f\n", pin_nr, *val);
    return 0;
}

void board_set_pin_modes(const pin_mode_nr_t new_modes[NUM_PINS]) {
    for (int i=0; i<NUM_PINS; ++i) {
        board.state.modes[i] = new_modes[i];
        // TODO(marco): Figure out why setup function is not called
        (pin_modes[new_modes[i]].fn_set)(i);
    }
    utils_write_binary("modes", board.state.modes, sizeof(pin_mode_nr_t), NUM_PINS);
}

void board_to_html(char content[NUM_CHARS_HTML]) {
    char pins[NUM_CHARS_JS] = "const PINS = [";
    for (int i=0; i<NUM_PINS; ++i) {
        if (board.allowed_modes[i] == 0) { continue; }

        char pin[2];
        snprintf(pin, 2, "%d", i);
        char mode[3];
        snprintf(mode, 3, "%d", board.state.modes[i]);
        char allowed[3];
        snprintf(allowed, 3, "%d", board.allowed_modes[i]);

        char str[18] = "[";
        strcat(str, pin);
        strcat(str, ",");
        strcat(str, allowed);
        strcat(str, ",");
        strcat(str, mode);
        strcat(str, "],");

        strcat(pins, str);
    }
    strcat(pins, "];");


    snprintf(content, NUM_CHARS_HTML, HTML_TEMPLATE, pins);
}
