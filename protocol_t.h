
#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef struct protocol_op
    {
        int x;
    } protocol_op;

    typedef struct protocol_t protocol_t;

    protocol_t *protocol_create();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PROTOCOL_H
