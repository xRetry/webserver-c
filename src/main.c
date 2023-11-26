#include "mongoose.h"
#include <stdio.h>

#define QUOTE(...) #__VA_ARGS__

#define TEMPLATE_HTML QUOTE()
#define NUM_PINS 10
#define NUM_CHARS_JS 17 + NUM_PINS * 18

struct {
    int modes[NUM_PINS];
} state;

void save_config(int config[NUM_PINS]) {
    FILE *fptr = fopen("config", "wb");
    if (fptr == NULL) {
        printf("Error opening config file\n");
        return;
    }

    fwrite(config, sizeof(int), NUM_PINS, fptr);
    fclose(fptr);
}

void read_config(int config[NUM_PINS]) {
    FILE *fptr = fopen("config", "rb");
    if (fptr == NULL) {
        for (int i=0; i<NUM_PINS; ++i) {
            config[i] = 0;
        }
        return;
    }

    int ret = fread(config, sizeof(int), NUM_PINS, fptr);
    if (ret != NUM_PINS) {
        printf("Error reading file: %d\n", ret);
    }
    fclose(fptr);
}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {

    char pins[NUM_CHARS_JS] = "const PINS = [";
    for (int i=0; i<NUM_PINS; ++i) {
        //TODO(marco): Insert condition for invalid pins
        if (false) { continue; }
        printf("%d\n", state.modes[i]);

        char pin[2];
        snprintf(pin, 2, "%d", i);
        char mode[3];
        snprintf(mode, 3, "%d", state.modes[i]);

        char str[18] = "[";
        strcat(str, pin);
        strcat(str, ",");
        strcat(str, "31");
        strcat(str, ",");
        strcat(str, mode);
        strcat(str, "],");

        strcat(pins, str);
    }
    strcat(pins, "];");

    char template[] = QUOTE(
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <title>Board Configuration</title>
                <style>
                    form {
                        display: grid;
                        grid-template-columns: 1fr 2fr;
                        gap: 10px;
                        width: 300px;
                        margin-top: 30px;
                        margin-left: auto;
                        margin-right: auto;
                    }
                </style>
            </head>
            <body>
                <form method="post" action="set-config">
                    <input type="submit" value="Save"/>
                </form>
            <script>
                %s
                const MODE_DESCS = ['Disabled', 'Digital Input', 'Digital Output', 'Analog Input', 'Analog Output'];

                let form = document.querySelector('form');
                for (const [num, modes, mode] of PINS.reverse()) {
                    let select = `<select name="${num}">`;
                    let i = 0;
                    for (const m of modes.toString(2)) {
                        const sel = i === mode ? ' selected' : '';
                        if (m > 0) { select += `<option value="${i}"${sel}>${MODE_DESCS[i]}</option>`; }
                        ++i;
                    }

                    form.insertAdjacentHTML('afterbegin', select+'</select>');
                    form.insertAdjacentHTML('afterbegin', `<label>Pin ${num}</label>`);
                }
            </script>
            </body>
        </html>
    );

    char content[strlen(template)+NUM_CHARS_JS];
    snprintf(content, strlen(template)+NUM_CHARS_JS, template, pins);

    mg_http_reply(
        conn, 
        200, 
        "Content-Type: text/html\r\n", 
        content
    );
};

static void handle_set_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct mg_str body = hm->body;
    
    for (int i=0; i<NUM_PINS; ++i) {
        char key[3];
        snprintf(key, 3, "%d", i);

        int mode = 0;
        struct mg_str val = mg_http_var(body, mg_str(key));
        if (val.len > 0) {
            char* end;
            mode = strtol(val.ptr, &end, 10);

        }
        state.modes[i] = mode;
        printf("%d - %d\n", i, mode);
    }

    save_config(state.modes);

    mg_http_reply(
        conn, 
        303, // `See Other`
        "Location: /config\r\n", 
        ""
    );
}

static void handle_pins_read(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    printf("read %s", ws_msg->data.ptr);
}

static void handle_pins_write(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    printf("write: %s", ws_msg->data.ptr);
}

static void router(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/config")) {
            handle_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/set-config")) {
            handle_set_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/pins/read")) {
            c->data[0] = 'r';
            mg_ws_upgrade(c, hm, NULL);
        } else if (mg_http_match_uri(hm, "/pins/write")) {
            c->data[0] = 'w';
            mg_ws_upgrade(c, hm, NULL);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/html", "");
        }
    } else if (ev == MG_EV_WS_MSG) {
        if (c->data[0] == 'r') {
            handle_pins_read(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'w') {
            handle_pins_write(c, ev, ev_data, fn_data);
        }
    }
}

void init_state() {
    read_config(state.modes);
}

int main() {
    init_state();

    struct mg_mgr mgr;                                
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", router, NULL);
    for (;;) {
        mg_mgr_poll(&mgr, 300);
    }

    return 0;
}

