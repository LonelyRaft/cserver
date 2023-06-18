
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "xlog.h"
#include "nd_pool_t.h"
#include "server_t.h"

static server_t *server;

int main() {
    xlog_init();
    node_pool_init(1024);
    server = server_create(1024);
    if (server != NULL) {
        server_start(server);
        while (server_running(server)) {
            sleep(1);
        }
    }
    node_pool_deinit();
    xlog_deinit();
    return 0;
}
