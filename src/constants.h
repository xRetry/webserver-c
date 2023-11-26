#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stdint.h>

#define NUM_PINS 10

#define NUM_CHARS_JS 17 + NUM_PINS * 18

#define QUOTE(...) #__VA_ARGS__

typedef uint8_t pin_mode_t;
typedef uint8_t pin_t;
typedef uint8_t err_t;

#define HTML_TEMPLATE "\
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
            <form method=\"post\" action=\"set-config\">\
                <input type=\"submit\" value=\"Save\"/>\
            </form>\
        <script>\
            %s\
            const MODE_DESCS = ['Disabled', 'Digital Input', 'Digital Output', 'Analog Input', 'Analog Output'];\
            let form = document.querySelector('form');\
            for (const [num, modes, mode] of PINS.reverse()) {\
                let select = `<select name=\"${num}\">`;\
                let i = 0;\
                for (const m of modes.toString(2).split('').reverse()) {\
                    const sel = i === mode ? ' selected' : '';\
                    if (m > 0) { select += `<option value=\"${i}\"${sel}>${MODE_DESCS[i]}</option>`; }\
                    ++i;\
                }\
                form.insertAdjacentHTML('afterbegin', select+'</select>');\
                form.insertAdjacentHTML('afterbegin', `<label>Pin ${num}</label>`);\
            }\
        </script>\
        </body>\
    </html>\
"

#define NUM_CHARS_HTML strlen(HTML_TEMPLATE)+NUM_CHARS_JS

#endif
