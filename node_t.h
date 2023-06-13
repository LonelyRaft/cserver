#ifndef NODE_T_H
#define NODE_T_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef struct node_t
    {
        void *data;
        struct node_t *next;
    } node_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // NODE_T_H
