#include <stdio.h>
#include "pin_modes.h"

err_t set_disabled(pin_mode_nr_t pin_mode_nr) {
    return 0;
}

err_t disabled(pin_nr_t pin_nr, double *val) {
    *val = 0;
    return 0;
}

err_t digital_write(pin_nr_t pin_nr, double *val) {
    printf("digital_write: pin=%d, val=%f", pin_nr, *val);
    return 0;
}

err_t set_digital_write(pin_nr_t pin_nr) {
    printf("set digital_write: pin=%d", pin_nr);
    return 0;
}

err_t digital_read(pin_nr_t pin_nr, double *val) {
    printf("set digital_read: pin=%d", pin_nr);
    return 0;
}

err_t set_digital_read(pin_nr_t pin_nr) {
    printf("set digital_read: pin=%d", pin_nr);
    return 0;
}

struct pin_mode_t pin_modes[] = {
    REGISTER_MODE("Disabled", set_digital_write, digital_write, ALLOWED_PINS(0,1,2,3,4,5,6,7,8,9)),
    REGISTER_MODE("Digital Write", set_digital_write, digital_write, ALLOWED_PINS(1, 3)),
    REGISTER_MODE("Digital Read", set_digital_read, digital_read, ALLOWED_PINS(4, 5)),
};

const int NUM_MODES = sizeof(pin_modes)/sizeof(struct pin_mode_t);
