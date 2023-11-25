#include "mongoose.h"
#include <stdio.h>

#define QUOTE(...) #__VA_ARGS__
#define NUM_PINS 10

void save_config(int config[NUM_PINS]) {
    FILE *fptr = fopen("config", "w");
    for (int i=0; i<NUM_PINS; ++i) {
        char mode[3];
        snprintf(mode, 3, "%d\n", config[i]);

        fprintf(fptr, mode, 3);
    }
    fclose(fptr);
}

void read_config(int config[NUM_PINS]) {
    FILE *fptr = fopen("config", "r");
    if (fptr == NULL) {
        for (int i=0; i<NUM_PINS; ++i) {
            config[i] = 0;
        }
        return;
    }

    int i = 0;
    char *line[3];
    size_t len = 0;
    ssize_t read;
    while ((read = getline(line, &len, fptr)) != -1) {
        char* end;
        config[i] = strtol(*line, &end, 10);
        ++i;
    }
    fclose(fptr);
}

static void handle_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    mg_http_reply(
        conn, 
        200, 
        "Content-Type: text/html\r\n", 
        QUOTE(
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
                        <label>Pin 1</label>
                        <select name="1">
                            <option value="0">Disabled</option>
                            <option value="1">Digital Input</option>
                            <option value="2">Digital Output</option>
                            <option value="3">Analog Input</option>
                            <option value="4">Analog Output</option>
                        </select>
                        <label>Pin 2</label>
                        <select name="2">
                            <option value="0">Disabled</option>
                            <option value="1">Digital Input</option>
                            <option value="2">Digital Output</option>
                            <option value="3">Analog Input</option>
                            <option value="4">Analog Output</option>
                        </select>
                        <input type="submit" value="Save"/>
                    </form>
                </body>
            </html>
        )
    );
};


static void handle_set_config(struct mg_connection *conn, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct mg_str body = hm->body;
    
    int config[NUM_PINS];
    for (int i=0; i<NUM_PINS; ++i) {
        char key[3];
        snprintf(key, 3, "%d", i);

        int mode = 0;
        struct mg_str val = mg_http_var(body, mg_str(key));
        if (val.len > 0) {
            char* end;
            mode = strtol(val.ptr, &end, 10);

        }
        config[i] = mode;
        printf("%d - %d\n", i, mode);
    }

    save_config(config);

    int config2[NUM_PINS];
    read_config(config2);

    for (int i=0; i<NUM_PINS; ++i) {
        printf("pin %d: %d\n", i, config2[i]);
    }

    mg_http_reply(
        conn, 
        303, 
        "Location: /config\r\n", 
        ""
    );
}


static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/config")) {
            handle_config(c, ev, ev_data, fn_data);
        } else if (mg_http_match_uri(hm, "/set-config")) {
            handle_set_config(c, ev, ev_data, fn_data);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/html", "");
        }
    }
}

int main() {

    struct mg_mgr mgr;                                
    mg_mgr_init(&mgr);                                      // Init manager
    mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL);  // Setup listener
    for (;;) mg_mgr_poll(&mgr, 1000);                       // Infinite event loop

    return 0;
}

