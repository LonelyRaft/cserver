
#ifndef _CCLIENT_H
#define _CCLIENT_H

#include <stddef.h>
#include "list_t.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifdef _WIN32
#include <winsock2.h>

typedef SOCKET skt_t;
#endif

#ifdef __linux__
#include <netinet/in.h>
typedef int skt_t;
#endif

typedef struct client_t
{
    skt_t sktfd;
    size_t length;
    struct sockaddr_in addr;
} client_t;

list_t *client_list_create();

int client_run(client_t *_client);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //  _CCLIENT_H
