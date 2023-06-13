

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

    typedef int skt_t;
#endif

    typedef struct client_t
    {
        skt_t sktfd;
        //    struct sockaddr_in addr;
        size_t length;
    } client_t;

    int apply_operation(list_t *_list);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //  _CCLIENT_H
