#include "server_t.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sktop.h"
#include "client_t.h"
#include "task_t.h"
#include "list_t.h"
#include "xlog.h"

typedef struct server_t
{
    pthread_t task_id;
    void *task_ret;
    unsigned short port;
    unsigned char run;
    unsigned char interval;
    task_vec_t *thds;
} server_t;

static list_t *server_q_find(server_t *_server)
{
    int full_ret = task_vec_full(_server->thds);
    if (full_ret == 1) {
        size_t vecsz =
            task_vec_capacity(_server->thds);
        if (vecsz == 0) {
            return NULL;
        }
        size_t idx = 1;
        const task_t *dest =
            task_vec_at(_server->thds, 0);
        while (idx < vecsz) {
            const task_t *curr =
                task_vec_at(_server->thds, idx);
            if (curr == NULL ||
                curr->entry != client_run) {
                continue;
            }
            size_t curr_num =
                list_count((list_t *)curr->arg);
            size_t dest_num =
                list_count((list_t *)dest->arg);
            if (dest_num > curr_num) {
                dest = curr;
            }
            idx++;
        }
        if (dest->arg != NULL) {
            xlogInfo("Find Dest Client Queue");
        } else {
            xlogError("Don't Find Dest Client Queue");
        }
        return (list_t *)dest->arg;
    } else if (full_ret != 0) {
        return NULL;
    } else {
        list_t *clnt_queue = client_list_create();
        if (clnt_queue == NULL) {
            xlogError("Create Client Queue Failed!");
            return NULL;
        }
        if (task_vec_push(_server->thds,
                client_run, clnt_queue)) {
            list_destroy(clnt_queue);
            xlogError("Push Client Task Failed!");
            return NULL;
        }
        xlogInfo("Create a Client Thread");
        return clnt_queue;
    }
}

static int server_recv_clnt(
    server_t *_server, skt_t sktfd)
{
    if (_server == NULL ||
        sktfd == INVALID_SOCKET) {
        return -1;
    }
    struct timeval tmout;
    tmout.tv_sec = 1;
    tmout.tv_usec = 0;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sktfd, &read_fds);
    while (_server->run) {
        int result = select(sktfd + 1, &read_fds, NULL, NULL, &tmout);
        if (result < 0) {
            xlogError("Select Error!");
            return -2;
        }
        if (result == 0) {
            tmout.tv_sec = 1;
            tmout.tv_usec = 0;
            continue;
        }
        if (!FD_ISSET(sktfd, &read_fds)) {
            continue;
        }
        struct sockaddr_in addr = {0};
        socklen_t length = sizeof(struct sockaddr_in);
        skt_t clnt_fd = accept(sktfd,
                (struct sockaddr *)&addr, &length);
        if (clnt_fd == INVALID_SOCKET) {
            xlogError("Accept Invalid Client Socket!");
            return -4;
        }
        if (socket_unblock(clnt_fd)) {
            socket_close(clnt_fd);
            xlogError("Set Unblock for Client Failed!");
            continue;
        }
        client_t new_clnt;
        new_clnt.sktfd = clnt_fd;
        new_clnt.addr = addr;
        new_clnt.length = length;
        list_t *queue = server_q_find(_server);
        if (queue == NULL) {
            socket_close(clnt_fd);
            xlogError("Client Queue Is Unavailable!");
            continue;
        }
        list_push(queue, &new_clnt);
        saddr_t server_addr = {0};
        socket_inaddr2saddr(&addr, &server_addr);
        xlogInfo("Accept Client: "
            "{ip:\"%u.%u.%u.%u\", port:%u}",
            server_addr.ipaddr.bs[3], server_addr.ipaddr.bs[2],
            server_addr.ipaddr.bs[1], server_addr.ipaddr.bs[0],
            server_addr.port);
        tmout.tv_sec = 1;
        tmout.tv_usec = 0;
    }
    return 0;
}

static void *server_run(server_t *_server)
{
    if (_server == NULL) {
        return (void *)(-1);
    }
    skt_t sktfd = INVALID_SOCKET;
    while (_server->run) {
        sktfd = socket(AF_INET,
                SOCK_STREAM, IPPROTO_TCP);
        if (sktfd == INVALID_SOCKET) {
            sleep(_server->interval);
            continue;
        }
        socket_reuseaddr(sktfd);
        struct sockaddr_in addr = {0};
        saddr_t server_addr = {0};
        server_addr.family = AF_INET;
        server_addr.port = _server->port;
        server_addr.ipaddr.addr = INADDR_ANY;
        socket_saddr2inaddr(&server_addr, &addr);
        if (bind(sktfd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_in))) {
            socket_close(sktfd);
            sleep(_server->interval);
            continue;
        }
        if (listen(sktfd, SOMAXCONN)) {
            socket_close(sktfd);
            sleep(_server->interval);
            continue;
        }
        while (_server->run) {
            if (server_recv_clnt(_server, sktfd)) {
                socket_close(sktfd);
                sleep(_server->interval);
                break;
            }
        }
    }
    if (sktfd != INVALID_SOCKET) {
        socket_close(sktfd);
    }
    xlogInfo("Server Thread Exit!");
    return NULL;
}

server_t *server_create(unsigned short _port)
{
    server_t *server =
        (server_t *)malloc(sizeof(server_t));
    if (server == NULL) {
        xlogError("Alloc Memory Failed!");
        return server;
    }
    server->port = _port;
    server->run = 0;
    server->interval = 1;
    server->thds =
        task_vec_create();
    if (server->thds == NULL) {
        xlogError("Create Client Thread Pool Failed!");
        free(server);
        server = NULL;
    }
    xlogInfo("Create Server With Port %d", _port);
    return server;
}

int server_destroy(server_t *_server)
{
    if (_server == NULL) {
        xlogWarn("This Is a Null Server!");
        return 0;
    }
    server_stop(_server);
    unsigned short port = _server->port;
    free(_server);
    xlogInfo("Free Server With Port %d", port);
    return 0;
}

int server_start(server_t *_server)
{
    if (_server == NULL) {
        xlogError("This Is a Null Server!");
        return -1;
    }
    _server->run = 1;
    pthread_create(&_server->task_id, NULL,
        (void *(*)(void *))server_run, _server);
    xlogInfo("Start Server(%d)", _server->port);
    return 0;
}

int server_stop(server_t *_server)
{
    if (_server == NULL) {
        xlogWarn("This Is a Null Server!");
        return 0;
    }
    // stop listen thread
    _server->run = 0;
    // stop client thread
    size_t vecsz =
        task_vec_capacity(_server->thds);
    size_t idx = 0;
    while (idx < vecsz) {
        const task_t *task =
            task_vec_at(_server->thds, idx);
        if (task == NULL ||
            task->entry != client_run) {
            idx++;
            continue;
        }
        task_vec_remove(_server->thds, idx);
        pthread_join(task->id, NULL);
        list_destroy((list_t *)task->arg);
        idx++;
    }
    if (_server->task_id)
        pthread_join(_server->task_id,
            &_server->task_ret);
    return 0;
}

int server_running(server_t *_server)
{
    if (_server == NULL) {
        return 0;
    }
    return _server->run;
}
