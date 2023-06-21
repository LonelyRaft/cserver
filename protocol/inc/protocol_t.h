
#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct protocol_op
{
    int x;
} protocol_op;

typedef struct protocol_t
{
    const protocol_op * op;
    unsigned char *recvbuf;
    unsigned char *sendbuf;
    size_t recvsz;
    size_t sendsz;
    size_t recv_widx;
    size_t send_widx;
    size_t recv_ridx;
    size_t send_ridx;
} protocol_t;

protocol_t *protocol_create();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROTOCOL_H
