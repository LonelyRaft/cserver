
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "pool_t.h"
#include "client_t.h"
#include "xlog.h"

static pool_t *client_pool = NULL;

int client_pool_create(size_t _capacity)
{
    if (_capacity == 0) {
        return -1;
    }
    if (client_pool == NULL) {
        client_pool = pool_create(
                sizeof(client_t), _capacity);
    }
    if (client_pool == NULL) {
        return -2;
    }
    return 0;
}

void client_pool_destroy()
{
    if (client_pool) {
        pool_destroy(client_pool);
        client_pool = NULL;
    }
}

static client_t *client_create(client_t *_client)
{
    if (_client == NULL) {
        return NULL;
    }
    client_t *clnt = (client_t *)ele_lease(client_pool);
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
        ele_release(client_pool, _client);
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

void *client_run(task_t *_task)
{
    if (_task == NULL ||
        _task->arg == NULL) {
        return NULL;
    }
    list_t *queue = (list_t *)_task->arg;
    while (_task->working) {
        client_t *clnt = (client_t *)list_begin(queue);
        while (clnt != NULL) {
            xlogStatus("Client Socket: %d\n", clnt->sktfd);
            clnt = (client_t *)list_next(queue);
        }
        sleep(1);
    }
    xlogStatus("Client Task Exiting!");
    return NULL;
}
