#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdint.h>

#define NUM_PINS 10

typedef uint8_t pin_mode_nr_t;
typedef uint8_t pin_nr_t;
typedef uint8_t err_t;

#define TEMPLATE_HTML_CONFIG "\
<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
    <title>Board Configuration</title>\
    <style>\
        form {\
            display: grid;\
            grid-template-columns: 1fr 2fr;\
            gap: 10px;\
            width: 300px;\
            margin-top: 30px;\
            margin-left: auto;\
            margin-right: auto;\
        }\
    </style>\
</head>\
<body>\
    <form method=\"post\" action=\"/api/config\">\
        <input type=\"submit\" value=\"Save\"/>\
    </form>\
<script>\
    const modes = %s;\
    const active = %s;\
    function build_selects(modes) {\
        const form = document.querySelector('form');\
\
        let pins = Array.from(new Set(modes.flatMap((mode) => mode.pins))).sort();\
        for (const pin of pins.reverse()) {\
            form.insertAdjacentHTML('afterbegin', `<select name=\"${pin}\"><select>`);\
            form.insertAdjacentHTML('afterbegin', `<label>Pin ${pin}</label>`);\
        }\
\
        modes.sort((a, b) => b.mode_nr - a.mode_nr);\
        for (const mode of modes) {\
            for (const pin of mode.pins) {\
                const select = document.querySelector(`select[name=\"${pin}\"]`);\
                select.insertAdjacentHTML('afterbegin', `<option value=\"${mode.mode_nr}\">${mode.name}</option>`);\
            }\
        }\
    }\
\
    function set_active(active) {\
        for (const [pin, mode] of Object.entries(active)) {\
            let select = document.querySelector(`select[name=\"${pin}\"]`);\
            if (select != null) { select.value = mode; }\
        }\
    }\
\
    build_selects(modes);\
    set_active(active);\
</script>\
</body>\
</html>\
"

#define TEMPLATE_JSON_MODE "{ \"mode_nr\": %d, \"name\": \"%s\", \"pins\": %s }"

#define OK(x) x == 0
#define ERR(x) !(OK(x))

#endif
