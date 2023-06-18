
#ifndef _CCLIENT_H
#define _CCLIENT_H

#include <stddef.h>
#include "list_t.h"
#include "sktop.h"

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

list_t *client_list_create();

void *client_run(list_t *_queue);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //  _CCLIENT_H
