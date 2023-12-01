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

static void handle_pins_json(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_ws_message *ws_msg = (struct mg_ws_message *) ev_data;
    
    int offset, length;
    if ((offset = mg_json_get(ws_msg->data, "$.write", &length)) >= 0) {
        struct mg_str write = mg_str_n(&ws_msg->data.ptr[offset], length);

        struct mg_str key, val;
        offset = 0;
        while ((offset = mg_json_next(write, offset, &key, &val)) > 0) {
            if (val.ptr == NULL) { continue; }

            long pin_nr;
            if (utils_string_to_long(key.ptr+1, &pin_nr)) { continue; }

            double pin_val;
            if (utils_string_to_double(val.ptr, &pin_val)) { continue; }

            board_pin_operation(pin_nr, &pin_val);
        }
    }

    char response[20*NUM_PINS+3] = "{";

    bool isFirst = true;
    offset = mg_json_get(ws_msg->data, "$.read", &length);
    if (offset >= 0) {
        struct mg_str read = mg_str_n(&ws_msg->data.ptr[offset], length);

        struct mg_str key, val;
        offset = 0;
        while ((offset = mg_json_next(read, offset, &key, &val)) > 0) {
            if (val.ptr == NULL) { continue; }

            long pin_nr;
            if (utils_string_to_long(val.ptr, &pin_nr) > 0) { continue; }


            double pin_val;
            board_pin_operation(pin_nr, &pin_val);

            char fmt[] = ",\"%d\":%.10g";
            if (isFirst) {
                fmt[0] = ' ';
                isFirst = false;
            }

            char buf[20]; 
            snprintf(buf, sizeof(buf), fmt, pin_nr, pin_val);
            strcat(response, buf);
        }
    }

    strcat(response, " }");

    mg_ws_send(c, response, strlen(response), WEBSOCKET_OP_TEXT);

}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    char content[LEN_HTML_TEMPLATE];
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

    
    pin_mode_nr_t modes[NUM_PINS];
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
    }

    board_init_pin_modes(modes);

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
        return;
    }
    conn->data[0] = direction;
    // pins is bigger than 1 byte, but the overflow is intented
    conn->data[1] = pins;
    mg_ws_upgrade(conn, hm, NULL);
}

static void router(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
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
        } else if (mg_http_match_uri(hm, "/pins/json")) {
            c->data[0] = 'j';
            mg_ws_upgrade(c, hm, NULL);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/html", "");
        }
    } else if (ev == MG_EV_WS_MSG) {
        if (c->data[0] == 'r') {
            handle_pins_read(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'w') {
            handle_pins_write(c, ev, ev_data, fn_data);
        } else if (c->data[0] == 'j') {
            handle_pins_json(c, ev, ev_data, fn_data);
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
