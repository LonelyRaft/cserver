
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    if (_a == _b) {
        return 0;
    }
    if (_a->sktfd == _b->sktfd) {
        return 0;
    }
    if (_a->sktfd > _b->sktfd) {
        return 1;
    }
    return -1;
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
        fd_set read;
        FD_ZERO(&read);
        int maxfd = 0;
        client_t *clnt = (client_t *)list_begin(queue);
        while (clnt != NULL) {
            if (clnt->sktfd == INVALID_SOCKET) {
                list_remove(queue, clnt);
                clnt = (client_t *)list_next(queue);
                continue;
            }
            FD_SET(clnt->sktfd, &read);
            if (clnt->sktfd > maxfd) {
                maxfd  = clnt->sktfd;
            }
            clnt = (client_t *)list_next(queue);
        }
        if (maxfd == 0) {
            sleep(1);
            continue;
        }
        struct timeval tmout;
        tmout.tv_sec = 1;
        tmout.tv_usec = 0;
        int result = select(maxfd + 1, &read, NULL, NULL, &tmout);
        if (result < 0) {
            xlogError("");
            break;
        }
        if (result == 0) {
            continue;
        }
        clnt = (client_t *)list_begin(queue);
        while (clnt != NULL) {
            if (clnt->sktfd == INVALID_SOCKET ||
                !FD_ISSET(clnt->sktfd, &read)) {
                clnt = (client_t *)list_next(queue);
                continue;
            }
            char buffer[256];
            int length = recv(clnt->sktfd, buffer, 255, 0);
            if (length < 0) {
                list_remove(queue, clnt);
                clnt = (client_t *)list_next(queue);
                continue;
            }
            buffer[length] = 0;
            xlogStatus("%s", buffer);
            send(clnt->sktfd, buffer, length, 0);
            // protocol do something
            clnt = (client_t *)list_next(queue);
        }
    }
    xlogStatus("Client Task Exiting!");
    return NULL;
}
