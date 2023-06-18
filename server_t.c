
#include "server_t.h"
#include <pthread.h>
#include <stdlib.h>
#include "client_t.h"
#include "thd_pool_t.h"
#include "list_t.h"
#include "xlog.h"

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET (-1)
#endif // INVALID_SOCKET

typedef union ip4addr_t {
    u_char bs[4];
    uint32_t addr;
} ip4addr_t;

typedef struct server_t
{
    unsigned short port;
    unsigned char run;
    thd_pool_t *thds;
} server_t;

static void *server_run(server_t *_server);

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
    //    thd_pool_get()
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

static int server_q_push(
    list_t *_server, client_t *_client)
{
}

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
    fd_set error_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(sktfd, &read_fds);
    FD_SET(sktfd, &error_fds);
    while (_server->run) {
        int result = select(sktfd + 1, &read_fds, NULL, &error_fds, NULL);
        if (result < 0) {
            return -2;
        } else {
            if (FD_ISSET(sktfd, &error_fds)) {
                return -3;
            }
            if (!FD_ISSET(sktfd, &read_fds)) {
                continue;
            }
            struct sockaddr_in addr = {0};
            socklen_t length = 0;
            skt_t clnt_fd = accept(sktfd,
                    (struct sockaddr *)&addr, &length);
            if (clnt_fd == INVALID_SOCKET) {
                return -4;
            }
            ip4addr_t clnt_ipaddr;
#ifdef _WIN32
            clnt_ipaddr.addr = addr.sin_addr.S_un.s_addr;
#endif // _WIN32
#ifdef __linux__
            clnt_ipaddr.addr = addr.sin_addr.s_addr;
#endif // __linux__
            xlogInfo("Accept Client: "
                "{ip:\"%03d.%03d.%03d.%03d\", "
                "port:%05d}",
                clnt_ipaddr.bs[0], clnt_ipaddr.bs[1],
                clnt_ipaddr.bs[2], clnt_ipaddr.bs[3],
                addr.sin_port);
            client_t new_clnt;
            new_clnt.sktfd = clnt_fd;
            new_clnt.addr = addr;
            new_clnt.length = length;
            list_t *queue = server_q_find(_server);
#ifdef _WIN32
            if (queue == NULL) {
                closesocket(clnt_fd);
                continue;
            }
#endif
#ifdef __linux__
            if (queue == NULL) {
                close(clnt_fd);
                continue;
            }
#endif
            xlogInfo("Append Client To Thread %d", dest_thd.id);
        }
    }
}

#ifdef __linux__
    #include <unistd.h>
    #include <sys/socket.h>
#endif // __linux__

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
#ifdef _WIN32
        BOOL opt = TRUE;
        setsockopt(
            sktfd, SOL_SOCKET, SO_REUSEADDR,
            (char *)&opt, sizeof(BOOL));
#endif //  _WIN32
#ifdef __linux__
        int resue = 1;
        setsockopt(sktfd, SOL_SOCKET,
            SO_REUSEADDR, &resue, sizeof(resue));
#endif // __linux__
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

#if defined(__GNUC__) && defined(_WIN32)
__attribute__((constructor))
void soket_init()
{
    WORD version  = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(version, &data);
}
__attribute__((destructor))
void socket_deinit()
{
    WSACleanup();
}
#endif // __GNUC__ && _WIN32
