
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "xlog.h"
#include "nd_pool_t.h"
#include "server_t.h"
#include "client_t.h"

static server_t *server;

int main()
{
    setbuf(stderr, NULL);
    xlog_init();
    node_pool_init(1024);
    client_pool_create(1024);
    server = server_create(1024);
    if (server != NULL) {
        server_start(server);
        while (server_running(server)) {
            sleep(1);
        }
        server_destroy(server);
    }
    client_pool_destroy();
    node_pool_deinit();
    xlog_deinit();
    return 0;
}
