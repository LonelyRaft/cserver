
#include "sktop.h"

#ifdef __linux__
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

int socket_close(skt_t _sktfd)
{
    return close(_sktfd);
}

int socket_reuseaddr(skt_t _sktfd)
{
    int resue = 1;
    return setsockopt(_sktfd, SOL_SOCKET,
                      SO_REUSEADDR, &resue, sizeof(resue));
}

int socket_unblock(skt_t _sktfd)
{
    int flags = fcntl(_sktfd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }
    if (flags & O_NONBLOCK) {
        return 0;
    }
    flags |= O_NONBLOCK;
    return fcntl(_sktfd, F_SETFL, flags);
}

int socket_inaddr2saddr(
    const struct sockaddr_in *_inaddr,
    saddr_t *_saddr)
{
    if (_inaddr == NULL ||
        _saddr == NULL) {
        return -1;
    }
    _saddr->family = _inaddr->sin_family;
    _saddr->port = ntohs(_inaddr->sin_port);
    _saddr->ipaddr.addr =
        ntohl(_inaddr->sin_addr.s_addr);
    return 0;
}

int socket_saddr2inaddr(
    const saddr_t *_saddr,
    struct sockaddr_in *_inaddr)
{
    if (_saddr == NULL ||
        _inaddr == NULL) {
        return -1;
    }
    _inaddr->sin_family = _saddr->family;
    _inaddr->sin_port = htons(_saddr->port);
    _inaddr->sin_addr.s_addr =
        htonl(_saddr->ipaddr.addr);
    return 0;
}

#endif // __linux__

#ifdef _WIN32

#ifdef __GNUC__
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
#endif // __GNUC__

typedef int socklen_t;

int socket_close(skt_t _sktfd)
{
    return closesocket(_sktfd);
}

int socket_reuseaddr(skt_t _sktfd)
{
    BOOL opt = TRUE;
    return setsockopt(
        _sktfd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&opt, sizeof(BOOL));
}

int socket_unblock(skt_t _sktfd)
{
    u_long noblock = 1;
    return ioctlsocket(_sktfd, FIONBIO, &noblock);
}

int socket_inaddr2saddr(
    const struct sockaddr_in *_inaddr,
    saddr_t *_saddr)
{
    if (_inaddr == NULL ||
        _saddr == NULL) {
        return -1;
    }
    _saddr->family = _inaddr->sin_family;
    _saddr->port = ntohs(_inaddr->sin_port);
    _saddr->ipaddr.addr =
        ntohl(_inaddr->sin_addr.s_addr);
    return 0;
}

int socket_saddr2inaddr(
    const saddr_t *_saddr,
    struct sockaddr_in *_inaddr)
{
    if (_saddr == NULL ||
        _inaddr == NULL) {
        return -1;
    }
    _inaddr->sin_family = _saddr->family;
    _inaddr->sin_port = htons(_saddr->port);
    _inaddr->sin_addr.s_addr =
        htonl(_saddr->ipaddr.addr);
    return 0;
}

#endif // _WIN32
