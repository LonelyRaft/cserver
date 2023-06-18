
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "client_t.h"
#include "xlog.h"

static client_t *client_create(client_t *_client)
{
    if (_client == NULL) {
        return NULL;
    }
    client_t *clnt = (client_t *)malloc(sizeof(client_t));
    memset(clnt, 0, sizeof(client_t));
    *clnt = *_client;
    return clnt;
}

static void client_destroy(client_t *_client)
{
    if (_client) {
        if (_client->sktfd != INVALID_SOCKET) {
            socket_close(_client->sktfd);
            _client->sktfd = INVALID_SOCKET;
        }
        free(_client);
        _client = NULL;
    }
    return;
}

static void client_copy(client_t *_dest, const client_t *_src)
{
    return;
}

static int client_cmp(const client_t *_a, const client_t *_b)
{
    return 0;
}

list_t *client_list_create()
{
    static const data_op client_op = {
        DATA_CREATE client_create,
        DATA_DESTROY client_destroy,
        DATA_COPY client_copy,
        DATA_COMPARE client_cmp,
    };
    return list_create(&client_op);
}

void *client_run(list_t *_queue)
{
    if (_queue == NULL) {
        return NULL;
    }
    while (1) {
        client_t *clnt = (client_t *)list_begin(_queue);
        while (clnt != NULL) {
            xlogStatus("Client Socket: %d\n", clnt->sktfd);
            clnt = (client_t *)list_next(_queue);
        }
        sleep(1);
    }
    return NULL;
}
