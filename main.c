
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "xlog.h"
 #include "nd_pool_t.h"
#include "server_t.h"
#include "client_t.h"

int main()
{
    xlog_init();
    node_pool_init(1024);

//    server_create();
//    server_start();
//    sleep(100);
//    server_stop();
//    server_destroy();

    client_list_create();

    node_pool_deinit();
    xlog_deinit();
    return 0;
}
