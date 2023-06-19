
#ifndef _SOCKET_OPERATION_H
#define _SOCKET_OPERATION_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef union ip4addr_t {
    unsigned char bs[sizeof(unsigned int)];
    unsigned int  addr;
} ip4addr_t;

typedef struct server_addr
{
    unsigned short family;
    unsigned short port;
    ip4addr_t ipaddr;
} saddr_t;

#ifdef _WIN32
#include <winsock2.h>

typedef SOCKET skt_t;
typedef int socklen_t;
#endif

#ifdef __linux__
#include <netinet/in.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif // INVALID_SOCKET

typedef int skt_t;
#endif

int socket_close(skt_t _sktfd);

int socket_reuseaddr(skt_t _sktfd);

int socket_unblock(skt_t _sktfd);

int socket_inaddr2saddr(
    const struct sockaddr_in *_inaddr,
    saddr_t *_saddr);

int socket_saddr2inaddr(
    const saddr_t *_saddr,
    struct sockaddr_in *_inaddr);

/*
_sktfd: unblock socket
*/
int socket_check(skt_t _sktfd);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _SOCKET_OPERATION_H
