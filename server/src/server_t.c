#include "server_t.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sktop.h"
#include "msg_t.h"
#include "client_t.h"
#include "task_t.h"
#include "list_t.h"
#include "xlog.h"

typedef struct server_t
{
    pthread_t task_id; // server task id
    skt_t sktfd; // server socket
    skt_t intf; // server interface
    saddr_t addr; // server addr
    unsigned char run; // server run flag
    unsigned char interval; // server run interval
    task_vec_t *thds; // client task pool
    list_t *clnts; // client queue
} server_t;

static list_t *server_list_find(server_t *_server)
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

static client_t *server_clnt_find(
    server_t *_server, skt_t _id)
{
    if (_server == NULL) {
        return NULL;
    }
    size_t vecsz =
        task_vec_capacity(_server->thds);
    if (vecsz == 0) {
        return NULL;
    }
    size_t idx = 0;
    while (idx < vecsz) {
        const task_t *task =
            task_vec_at(_server->thds, idx);
        if (task == NULL ||
            task->entry != client_run) {
            continue;
        }
        client_t *clnt  = (client_t *)list_next((list_t *)task->arg);
        while (clnt != NULL) {
            if (clnt->sktfd == _id) {
                return clnt;
            }
            clnt = (client_t *)list_next((list_t *)task->arg);
        }
        idx++;
    }
    return NULL;
}

static int server_fdset_push(
    server_t *_server, fd_set *_fdset)
{
    int maxfd = 0;
    if (_server == NULL || _fdset == NULL ||
        _server->sktfd == INVALID_SOCKET ||
        _server->clnts == NULL) {
        return maxfd;
    }
    maxfd = _server->sktfd;
    FD_SET(_server->sktfd, _fdset);
    client_t *clnt = (client_t *)list_begin(_server->clnts);
    while (clnt != NULL) {
        if (clnt->sktfd == INVALID_SOCKET) {
            list_remove(_server->clnts, clnt);
            clnt = (client_t *)list_next(_server->clnts);
            continue;
        }
        FD_SET(clnt->sktfd, _fdset);
        if (clnt->sktfd > maxfd) {
            maxfd  = clnt->sktfd;
        }
        clnt = (client_t *)list_next(_server->clnts);
    }
    return maxfd;
}

static int server_recv_clnt(server_t *_server)
{
    if (_server == NULL || _server->clnts == NULL) {
        return -1;
    }
    struct sockaddr_in addr = {0};
    socklen_t length = sizeof(struct sockaddr_in);
    skt_t clnt_fd = accept(_server->sktfd,
            (struct sockaddr *)&addr, &length);
    if (clnt_fd == INVALID_SOCKET) {
        xlogError("Accept Invalid Client Socket!");
        return -2;
    }
    if (socket_unblock(clnt_fd)) {
        socket_close(clnt_fd);
        xlogError("Set Unblock for Client Failed!");
        return -3;
    }
    client_t new_clnt = {0};
    new_clnt.sktfd = clnt_fd;
    new_clnt.addr = addr;
    new_clnt.intf = INVALID_SOCKET;
    if (list_push(_server->clnts, &new_clnt)) {
        xlogError("Push New Client To Client List Failed!");
        return -4;
    }
    saddr_t server_addr = {0};
    socket_inaddr2saddr(&addr, &server_addr);
    xlogInfo("Accept Client: "
        "{ip:\"%u.%u.%u.%u\", port:%u}",
        server_addr.ipaddr.bs[3], server_addr.ipaddr.bs[2],
        server_addr.ipaddr.bs[1], server_addr.ipaddr.bs[0],
        server_addr.port);
    return 0;
}

static int server_fdset_process(
    server_t *_server, fd_set *_fdset)
{
    int result = -1;
    if (_server == NULL || _fdset == NULL ||
        _server->sktfd == INVALID_SOCKET ||
        _server->clnts == NULL) {
        return result;
    }
    if (FD_ISSET(_server->sktfd, _fdset)) {
        result = server_recv_clnt(_server);
        if (result == -1 || result == -2) {
            return -2;
        }
    }
    result = 0;
    client_t *clnt = (client_t *)list_begin(_server->clnts);
    while (clnt != NULL) {
        if (clnt->sktfd == INVALID_SOCKET ||
            !FD_ISSET(clnt->sktfd, _fdset)) {
            clnt = (client_t *)list_next(_server->clnts);
            continue;
        }
        loc_msg_t locmsg = {0};
        int length = recv(clnt->sktfd,
                (char *)&locmsg, sizeof(loc_msg_t), 0);
        if (length != sizeof(loc_msg_t)) {
            list_remove(_server->clnts, clnt);
            clnt = (client_t *)list_next(_server->clnts);
            continue;
        }
        if (locmsg.type == SKT_TYPE_DATA) {
            clnt->type = SKT_TYPE_DATA;
            list_t *queue = server_list_find(_server);
            list_push(queue, clnt);
            if (_server->intf != INVALID_SOCKET) {
                length = send(_server->intf,
                        (char *)clnt, sizeof(client_t), 0);
                if (length != sizeof(client_t)) {
                    socket_close(_server->intf);
                    _server->intf = INVALID_SOCKET;
                }
            }
            clnt->sktfd = INVALID_SOCKET;
            list_remove(_server->clnts, clnt);
        } else if (locmsg.type == SKT_TYPE_INTF) {
            clnt->type = SKT_TYPE_INTF;
            client_t *data_clnt =
                server_clnt_find(_server, locmsg.id);
            if (data_clnt != NULL) {
                data_clnt->intf = clnt->sktfd;
            }
            clnt->sktfd = INVALID_SOCKET;
            list_remove(_server->clnts, clnt);
        } else if (locmsg.type == SKT_TYPE_LIST) {
            clnt->type = SKT_TYPE_LIST;
            _server->intf = clnt->sktfd;
            clnt->sktfd = INVALID_SOCKET;
            list_remove(_server->clnts, clnt);
        } else {
            list_remove(_server->clnts, clnt);
        }
        clnt = (client_t *)list_next(_server->clnts);
    }
    return result;
}

static int server_dispatch_clnt(server_t *_server)
{
    int result = 0;
    if (_server == NULL ||
        _server->sktfd ==
        INVALID_SOCKET) {
        result = -1;
        return result;
    }
    while (_server->run) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int maxfd = server_fdset_push(_server, &read_fds);
        if (maxfd == 0) {
            result = -2;
            break;
        }
        struct timeval tmout = {0};
        tmout.tv_sec = 1;
        tmout.tv_usec = 0;
        int result = select(maxfd + 1, &read_fds, NULL, NULL, &tmout);
        if (result < 0) {
            xlogError("Select Error!");
            result = -3;
            break;
        }
        if (result == 0) {
            continue;
        }
        if (server_fdset_process(_server, &read_fds)) {
            break;
        }
    }
    if (result != 0) {
        socket_close(_server->sktfd);
        _server->sktfd = INVALID_SOCKET;
    }
    return result;
}

static skt_t server_listen(server_t *_server)
{
    skt_t sktfd = INVALID_SOCKET;
    if (_server == NULL) {
        return sktfd;
    }
    do {
        sktfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sktfd == INVALID_SOCKET) {
            break;
        }
        socket_reuseaddr(sktfd);
        struct sockaddr_in addr = {0};
        socket_saddr2inaddr(&_server->addr, &addr);
        if (bind(sktfd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_in))) {
            socket_close(sktfd);
            sktfd = INVALID_SOCKET;
            break;
        }
        if (listen(sktfd, SOMAXCONN)) {
            socket_close(sktfd);
            sktfd = INVALID_SOCKET;
        }
    } while (0);
    _server->sktfd = sktfd;
    return sktfd;
}

static void *server_run(server_t *_server)
{
    if (_server == NULL) {
        return (void *)(-1);
    }
    while (_server->run) {
        if (server_listen(_server) ==
            INVALID_SOCKET) {
            sleep(_server->interval);
            continue;
        }
        while (_server->run) {
            if (server_dispatch_clnt(_server)) {
                sleep(_server->interval);
                break;
            }
        }
    }
    if (_server-> sktfd != INVALID_SOCKET) {
        socket_close(_server-> sktfd);
        _server-> sktfd = INVALID_SOCKET;
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
    memset(server, 0, sizeof(server_t));
    server->intf = INVALID_SOCKET;
    server->addr.family = AF_INET;
    server->addr.port = _port;
    server->addr.ipaddr.addr = INADDR_ANY;
    server->interval = 1;
    server->thds = task_vec_create(2);
    if (server->thds == NULL) {
        xlogError("Create Client Thread Pool Failed!");
        free(server);
        server = NULL;
        return server;
    }
    server->clnts = client_list_create();
    if (server->clnts == NULL) {
        xlogError("Create Client Queue Failed!");
        free(server);
        server = NULL;
        return server;
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
    unsigned short port = _server->addr.port;
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
    xlogInfo("Start Server(%d)", _server->addr.port);
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
    if (_server->task_id) {
        pthread_join(_server->task_id, NULL);
    }
    return 0;
}

int server_running(server_t *_server)
{
    if (_server == NULL) {
        return 0;
    }
    return _server->run;
}
