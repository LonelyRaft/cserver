
#ifndef _CCLIENT_H
#define _CCLIENT_H

#include <stddef.h>
#include "list_t.h"
#include "sktop.h"
#include "task_t.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct client_t
{
    skt_t sktfd;
    size_t length;
    struct sockaddr_in addr;
} client_t;

int client_pool_create(size_t _capacity);

void client_pool_destroy();

list_t *client_list_create();

void *client_run(task_t *_task);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //  _CCLIENT_H
