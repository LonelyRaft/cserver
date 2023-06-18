
#include "server_t.h"
#include <pthread.h>
#include "client_t.h"
#include "thd_pool_t.h"
#include "list_t.h"
#include "clog.h"

typedef struct server_t
{
    unsigned short port;
    unsigned char run;
    void *(*clnt_entry)(void *);
    thd_pool_t *thds;
} server_t;

static void *server_run(server_t *_server);

int server_create()
{
    return 0;
}

int server_destroy()
{
    return 0;
}

int server_start()
{
    //    pthread_create(0, 0, server_run, 0);
    return 0;
}

int server_stop()
{
    return 0;
}

int server_pause()
{
    return 0;
}

#ifdef _WIN32

static int server_recv_clnt(server_t *_server, skt_t sktfd)
{
    while (_server->run) {
        int length = 0;
        struct sockaddr_in addr = {0};
        skt_t skt_clnt = accept(
                sktfd, (struct sockaddr *)&addr, &length);
        if (skt_clnt == INVALID_SOCKET) {
            closesocket(sktfd);
            return -1;
        }
        thd_t dest_thd = {0};
        if (thd_pool_full(_server->thds)) {
            size_t thd_cnt = thd_pool_count(_server->thds);
            thd_t dest_thd = {0};
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
        } else {
            if (_server->clnt_entry == NULL) {
                closesocket(skt_clnt);
                continue;
            }
            dest_thd.entry = _server->clnt_entry;
            dest_thd.arg = client_list_create();
            if (dest_thd.arg == NULL) {
                closesocket(sktfd);
                continue;
            }
            pthread_create(&dest_thd.id, NULL, dest_thd.entry, dest_thd.arg);
            if (dest_thd.id == 0) {
                list_destroy((list_t *)dest_thd.arg);
                dest_thd.arg = NULL;
                closesocket(skt_clnt);
                continue;
            }
        }
        if (dest_thd.arg == NULL) {
            closesocket(skt_clnt);
            continue;
        }
        client_t *new_clnt = (client_t *)malloc(sizeof(client_t));
        if (new_clnt == NULL) {
            closesocket(skt_clnt);
            continue;
        }
        memset(new_clnt, 0, sizeof(client_t));
        new_clnt->sktfd = skt_clnt;
        new_clnt->length = length;
        new_clnt->addr = addr;
        list_push((list_t *)dest_thd.arg, new_clnt);
    }
    return 0;
}

static void *server_run(server_t *_server)
{
    int result = 0;
    if (_server == NULL) {
        return (void *)(-1);
    }
    while (_server->run) {
        skt_t sktfd = INVALID_SOCKET;
        sktfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sktfd == INVALID_SOCKET) {
            Sleep(1000);
            continue;
        }
        BOOL opt = TRUE;
        setsockopt(
            sktfd, SOL_SOCKET, SO_REUSEADDR,
            (char *)&opt, sizeof(BOOL));
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.S_un.S_addr =
            htonl(INADDR_ANY);
        addr.sin_port = htons(_server->port);
        if (bind(sktfd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_in))) {
            closesocket(sktfd);
            Sleep(1000);
            continue;
        }
        if (listen(sktfd, SOMAXCONN)) {
            closesocket(sktfd);
            Sleep(1000);
            continue;
        }
        while (_server->run) {
            if (server_recv_clnt(_server, sktfd)) {
                break;
            }
        }
    }
    return 0;
}

#endif //  _WIN32

#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h>

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET (-1)
#endif // INVALID_SOCKET

static int server_recv_clnt(server_t *_server, skt_t sktfd)
{
    if (_server == NULL ||
        sktfd == INVALID_SOCKET) {
        return -1;
    }
    while (_server->run) {
        struct sockaddr_in addr = {0};
        socklen_t length = 0;
        skt_t clnt_skt = accept(sktfd,
                (struct sockaddr *)&addr, &length);
        if (clnt_skt == INVALID_SOCKET) {
            return -2;
        }
    }
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
            sleep(1);
            continue;
        }
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(_server->port);
        if (bind(sktfd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr))) {
            close(sktfd);
            sleep(1);
            continue;
        }
        if (listen(sktfd, SOMAXCONN)) {
            close(sktfd);
            sleep(1);
            continue;
        }
        while (_server->run) {
            if (server_recv_clnt(_server, sktfd)) {
                close(sktfd);
                break;
            }
        }
    }
    return NULL;
}

#endif // __linux__
