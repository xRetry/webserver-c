#include <stdio.h>
#include <string.h>
#include "board.h"
#include "utils.h"

#include "pin_modes.h"

struct board_t {
    struct state_t {
        pin_mode_nr_t modes[NUM_PINS];
    } state;
    err_t (*rw_functions[NUM_PINS])(pin_nr_t, double*);

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
    err_t err = utils_read_binary("modes", board.state.modes, sizeof(pin_mode_nr_t), NUM_PINS);
    if (err != 0) {
        for (int i=0; i<NUM_PINS; ++i) {
            board.state.modes[i] = 0;
        }
    }
}

void modes_init(void) {
    for (int i=0; i<NUM_PINS; ++i) {
        struct pin_mode_t pin_mode = PIN_MODES[board.state.modes[i]];
        pin_mode.fn_init(i);
        board.rw_functions[i] = pin_mode.fn_rw;
    }
}

void board_init(void) {
    allowed_init();
    state_init();
    modes_init();
}

err_t board_pin_operation(pin_nr_t pin_nr, double *val) {
    if (pin_nr >= NUM_PINS) { return 1; }

    board.rw_functions[pin_nr](pin_nr, val);

    printf("pin op: %d - %f\n", pin_nr, *val);
    return 0;
}

void board_init_pin_modes(const pin_mode_nr_t new_modes[NUM_PINS]) {
    for (int i=0; i<NUM_PINS; ++i) {
        struct pin_mode_t pin_mode = PIN_MODES[new_modes[i]];
        if (ERR(pin_mode.fn_init(i))) { 
            // TODO(marco): Save errors
            continue; 
        };
        board.rw_functions[i] = pin_mode.fn_rw;
        board.state.modes[i] = new_modes[i];
    }
    utils_write_binary("modes", board.state.modes, sizeof(pin_mode_nr_t), NUM_PINS);
}

void utils_array_to_json(char *str, char *arr[], int arr_len, int str_len) {
    strcat(str, "[");
    for (int i=0; i<arr_len; ++i) {
        strcat(str, arr[i]);

        char delimiter[] = ",";
        if (i == arr_len-1) { delimiter[0] = '\0'; }
        strcat(str, delimiter);
    }
    strcat(str, "]");
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
        snprintf(mode, LEN_JS_PIN_STAT, "%d", board.state.modes[i]);

        char str[LEN_JS_PIN_STATS] = "";
        char *modes[] = {pin, allowed, mode};

        utils_array_to_json(str, modes, 3, 3);

        strcat(pins, str);
        strcat(pins, ",");
    }


    char names[LEN_JS_NAMES] = "";
    for (int i=0; i<NUM_MODES; ++i) {
        strcat(names, "\"");
        strcat(names, PIN_MODES[i].name);
        strcat(names, "\"");
        strcat(names, ",");
    }

    snprintf(content, LEN_HTML_TEMPLATE, HTML_TEMPLATE, pins, names);
}
