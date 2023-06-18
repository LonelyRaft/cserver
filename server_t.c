#include "server_t.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sktop.h"
#include "client_t.h"
#include "thd_pool_t.h"
#include "list_t.h"
#include "xlog.h"

typedef struct server_t
{
    unsigned short port;
    unsigned char run;
    unsigned char interval;
    thd_pool_t *thds;
} server_t;

static list_t *server_q_find(server_t *_server)
{
    if (thd_pool_full(_server->thds)) {
        thd_t dest_thd = {0};
        size_t thd_cnt = thd_pool_count(_server->thds);
        for (size_t idx = 0; idx < thd_cnt; idx++) {
            thd_t curr_thd = {0};
            if (thd_pool_get(_server->thds, idx, &curr_thd)) {
                continue;
            }
            size_t curr_num = list_count((list_t *)curr_thd.arg);
            size_t dest_num = list_count((list_t *)dest_thd.arg);
            if (dest_num > curr_num) {
                dest_thd = curr_thd;
            }
        }
        if (dest_thd.arg != NULL) {
            xlogInfo("Find Dest Client Queue");
        } else {
            xlogError("Don't Find Dest Client Queue");
        }
        return (list_t *)dest_thd.arg;
    } else {
        thd_t new_thd = {0};
        new_thd.entry = (void *(*)(void *))client_run;
        new_thd.arg = client_list_create();
        if (new_thd.arg == NULL) {
            xlogError("Create Cleint List Failed!");
            return NULL;
        }
        if (pthread_create(&new_thd.id, NULL, new_thd.entry, new_thd.arg)) {
            list_destroy((list_t *)new_thd.arg);
            xlogError("Create Client Thread Failed!");
            return NULL;
        }
        thd_pool_add(_server->thds, new_thd);
        xlogInfo("Create a Client Thread");
        return (list_t *)new_thd.arg;
    }
}

static int server_recv_clnt(
    server_t *_server, skt_t sktfd)
{
    if (_server == NULL ||
        sktfd == INVALID_SOCKET) {
        return -1;
    }
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sktfd, &read_fds);
    while (_server->run) {
        int result = select(sktfd + 1, &read_fds, NULL, NULL, NULL);
        if (result < 0) {
            xlogError("Select Error!");
            return -2;
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
    }
    return 0;
}

static void *server_run(server_t *_server)
{
    if (_server == NULL) {
        return (void *)(-1);
    }
    while (_server->run) {
        skt_t sktfd = INVALID_SOCKET;
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
    server->run = 1;
    server->thds =
        thd_pool_create();
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
    // stop listen thread
    _server->run = 0;
    // stop client thread
    size_t thdcnt = thd_pool_count(_server->thds);
    size_t thdidx = 0;
    while (thdidx < thdcnt) {
        thd_t todel = {0};
        thd_pool_get(_server->thds, thdidx, &todel);
        list_destroy((list_t*)todel.arg);
        pthread_cancel(todel.id);
        thdidx++;
    }
    thd_pool_destroy(_server->thds);
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
    pthread_t thd = 0;
    pthread_create(&thd, NULL,
        (void *(*)(void *))server_run, _server);
    xlogInfo("Start Server(%d)", _server->port);
    return 0;
}

int server_stop(server_t *_server)
{
    return 0;
}

int server_pause(server_t *_server)
{
    return 0;
}

int server_running(server_t *_server)
{
    if (_server == NULL) {
        return 0;
    }
    return _server->run;
}
