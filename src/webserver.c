#include "mongoose.h"
#include "webserver.h"
#include "constants.h"
#include "board.h"
#include "utils.h"

static void handle_pins_read(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;

    // TODO(marco): Allocate memory the correct number pins
    double vals[NUM_PINS];
    const long pin_mask = conn->data[1];
    printf("%d", pin_mask);
    for (int pin_nr=0; pin_nr<NUM_PINS; ++pin_nr) {
        const bool use_pin = (pin_mask >> pin_nr) & 1;
        if (use_pin) {
            board_pin_operation(pin_nr, &vals[pin_nr]);
        } else {
            vals[pin_nr] = 0.;
        }
    }

    for (int i=0; i<NUM_PINS; ++i) {
        printf("%f,", vals[i]);
    }
    printf("\nread %s\n", ws_msg->data.ptr);
}

static void handle_pins_write(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    printf("write: %s", ws_msg->data.ptr);
}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    char content[NUM_CHARS_HTML];
    board_to_html(content);
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
    
    pin_mode_t modes[NUM_PINS];
    for (int i=0; i<NUM_PINS; ++i) {
        char key[3];
        snprintf(key, 3, "%d", i);

        int mode = 0;
        struct mg_str val = mg_http_var(body, mg_str(key));
        if (val.len > 0) {
            char* end;
            mode = strtol(val.ptr, &end, 10);

        }
        modes[i] = mode;
        printf("%d - %d\n", i, mode);
    }

    board_set_pin_modes(modes);

    mg_http_reply(
        conn, 
        303, // Status code: `See Other`
        "Location: /config\r\n", 
        ""
    );
}

static void handle_ws_request(struct mg_connection *conn, struct mg_http_message *hm, char direction, const struct mg_str *params) {
    long pins;
    if (utils_string_to_long(params[0].ptr, &pins) != 0) {
        mg_http_reply(conn, 404, "Content-Type: text/html", "");
        printf("error\n");
        return;
    }
    printf("cp\n");
    conn->data[0] = direction;
    // pins is bigger than 1 byte, but the overflow is intented
    conn->data[1] = pins;
    mg_ws_upgrade(conn, hm, NULL);
}

static void router(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        printf("http\n");
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_str params[1];

        if (mg_http_match_uri(hm, "/config")) {
            handle_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/set-config")) {
            handle_set_config(c, ev, ev_data, fn_data);
        } else if (mg_match(hm->uri, mg_str("/pins/read/*"), params)) {
            handle_ws_request(c, hm, 'r', params);
        } else if (mg_match(hm->uri, mg_str("/pins/write/*"), params)) {
            handle_ws_request(c, hm, 'w', params);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/html", "");
        }
    } else if (ev == MG_EV_WS_MSG) {
        printf("ws\n");
        if (c->data[0] == 'r') {
            handle_pins_read(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'w') {
            handle_pins_write(c, ev, ev_data, fn_data);
        }
    }
}

void webserver_run(void) {
    struct mg_mgr mgr;                                
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", router, NULL);
    for (;;) {
        mg_mgr_poll(&mgr, 300);
    }
}
