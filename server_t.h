
#ifndef _SERVER_H
#define _SERVER_H

#ifdef __cpluplus
extern "C"
{
#endif // __cpluplus

    int server_create();

    int server_destroy();

    int server_start();

    int server_stop();

    int server_pause();

#ifdef __cpluplus
}
#endif // __cpluplus

#endif //  _SERVER_H
