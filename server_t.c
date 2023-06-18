
#include "server_t.h"
#include <pthread.h>
#include "client_t.h"
#include "thd_pool_t.h"
#include "list_t.h"
#include "xlog.h"

typedef struct server_t {
    unsigned short port;
    unsigned char run;
    void *(*clnt_entry)(void *);
    thd_pool_t *thds;
} server_t;

static void *server_run(server_t *_server);

server_t *server_create(unsigned short _port) {
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

int server_destroy(server_t *_server) {
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

int server_start(server_t *_server) {
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

int server_stop(server_t *_server) {
    return 0;
}

int server_pause(server_t *_server) {
    return 0;
}

int server_running(server_t *_server) {
    if (_server == NULL) {
        return 0;
    }
    return _server->run;
}


#ifdef _WIN32

static int server_recv_clnt(server_t *_server, skt_t sktfd) {
    while (_server->run) {
        int length = 0;
        struct sockaddr_in addr = {0};
        skt_t skt_clnt = accept(
                sktfd, (struct sockaddr *)&addr, &length);
        if (skt_clnt == INVALID_SOCKET) {
            xlogWarn("Accept Client Failed!");
            return -1;
        }
        xlogInfo(
            "Accept Client: {ip:\"%03d.%03d.%03d.%03d\", port:%05d}",
            addr.sin_addr.S_un.S_un_b.s_b1,
            addr.sin_addr.S_un.S_un_b.s_b2,
            addr.sin_addr.S_un.S_un_b.s_b3,
            addr.sin_addr.S_un.S_un_b.s_b4,
            addr.sin_port);
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
            xlogDebug("Find Appo Thread");
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
            xlogDebug("Create a Client Thread");
        }
        client_t *new_clnt =
            (client_t *)malloc(sizeof(client_t));
        if (new_clnt == NULL) {
            closesocket(skt_clnt);
            xlogError("Alloc Memory Failed!");
            continue;
        }
        memset(new_clnt, 0, sizeof(client_t));
        new_clnt->sktfd = skt_clnt;
        new_clnt->length = length;
        new_clnt->addr = addr;
        list_push((list_t *)dest_thd.arg, new_clnt);
        xlogInfo("Appen Client To Thread %d", dest_thd.id);
    }
    return 0;
}

static void *server_run(server_t *_server) {
    int result = 0;
    if (_server == NULL) {
        return (void *)(-1);
    }
    while (_server->run) {
        skt_t sktfd = INVALID_SOCKET;
        sktfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sktfd == INVALID_SOCKET) {
            Sleep(1000);
            continue;
        }
//        BOOL opt = TRUE;
//        setsockopt(
//            sktfd, SOL_SOCKET, SO_REUSEADDR,
//            (char *)&opt, sizeof(BOOL));
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_addr.S_un.S_addr = INADDR_ANY;
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
                closesocket(sktfd);
                Sleep(1000);
                break;
            }
        }
    }
    return 0;
}

#endif //  _WIN32

#ifdef __linux__

static int server_recv_clnt(server_t *_server, skt_t sktfd) {
}

static void *server_run(server_t *_server) {
}

#endif // __linux__

#if defined(__GNUC__) && defined(_WIN32)
__attribute__((constructor))
void soket_init() {
    WORD version  = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(version, &data);
}
__attribute__((destructor))
void socket_deinit() {
    WSACleanup();
}
#endif // __GNUC__ && _WIN32
