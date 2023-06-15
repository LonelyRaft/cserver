
#ifndef DATA_DEFINITION_H
#define DATA_DEFINITION_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define DATA_CREATE (void*(*)(void*))
#define DATA_DESTROY (int(*)(void*))
#define DATA_COPY (int (*)(void *, const void *))
#define DATA_COMPARE (int (*)(const void *, const void *))
#define DATA_PROCESS (int (*)(void *))

typedef struct data_op
{
    void* (*create)(void* _data); // copy create
    int (*destroy)(void* _data);
    int (*copy)(void* _dest, const void* _src);
    int (*cmp)(const void *_a, const void *_b);
    int (*process)(void *_data);
} data_op;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DATA_DEFINITION_H
