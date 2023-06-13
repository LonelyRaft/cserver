
#ifndef _LIST_H
#define _LIST_H

#ifdef __cpluplus
extern "C" {
#endif // __cpluplus

typedef struct list_data_op
{
    int (*copy)(void* _dest, void* _src);
    int (*equal)(void* _a, void * _b);
}list_data_op;

typedef struct list_t list_t;

#ifdef __cpluplus
}
#endif // __cpluplus

#endif //  _LIST_H
