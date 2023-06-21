
#ifndef _SERVER_H
#define _SERVER_H

#ifdef __cpluplus
extern "C"
{
#endif // __cpluplus

    typedef struct server_t server_t;

    server_t *server_create(unsigned short _port);

    int server_destroy(server_t *_server);

    int server_start(server_t *_server);

    int server_stop(server_t *_server);

    int server_running(server_t *_server);

#ifdef __cpluplus
}
#endif // __cpluplus

#endif //  _SERVER_H
