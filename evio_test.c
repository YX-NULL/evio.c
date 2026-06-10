#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "evio.h"

void serving(const char **addrs, int naddrs, void *udata) {
    printf("Server started, listening on:\n");
    for (int i = 0; i < naddrs; i++) {
        printf("   ➜ %s\n", addrs[i]);
    }
}

void error(const char *msg, bool fatal, void *udata) {
    fprintf(stderr, "Error: %s\n", msg);
    if (fatal) {
        exit(1);
    }
}

/*
int64_t tick(void *udata) {
    static int count = 0;
    printf("Heartbeat #%d\n", ++count);
    return 1000000000LL;
}
*/

void opened(struct evio_conn *conn, void *udata) {
    const char *addr = evio_conn_addr(conn);
    printf("Connection opened: %s\n", addr);
    
    const char *welcome = "Welcome! Type 'quit' to exit.\n";
    evio_conn_write(conn, welcome, strlen(welcome));
}

void closed(struct evio_conn *conn, void *udata) {
    const char *addr = evio_conn_addr(conn);
    printf("Connection closed: %s\n", addr);
}

void data(struct evio_conn *conn, const void *data, size_t len, void *udata) {
    char *msg = (char*)data;
    
    if (strncmp(msg, "quit", 4) == 0 && (len == 4 || msg[4] == '\n' || msg[4] == '\r')) {
        printf("Quit command received, closing connection.\n");
        evio_conn_close(conn);
        return;
    }
    
    printf("Received message (len %zu): %.*s", len, (int)len, msg);
    evio_conn_write(conn, data, len);
}

int main() {
    struct evio_events evs = {
        .serving = serving,
        .error = error,
       // .tick = tick,
        .opened = opened,
        .closed = closed,
        .data = data,
    };

    const char *addrs[] = {
        "tcp://0.0.0.0:8888",
        "unix://echo.sock"
    };
    int naddrs = sizeof(addrs) / sizeof(addrs[0]);

    evio_main(addrs, naddrs, evs, NULL);

    return 0;
}