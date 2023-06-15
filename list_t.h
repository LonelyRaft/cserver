
#ifndef _LIST_H
#define _LIST_H

#ifdef __cpluplus
extern "C"
{
#endif // __cpluplus

#define DATA_COPY (int (*)(void *, const void *))
#define DATA_EQUAL (int (*)(const void *, const void *))
#define DATA_PROCESS (int (*)(void *))

    typedef struct list_data_op
    {
        int (*copy)(void *_dest, const void *_src);
        int (*equal)(const void *_a, const void *_b);
        int (*process)(void *_data);
    } list_data_op;

    typedef struct list_t list_t;

    list_t *list_create();

    int list_clear(list_t *_list);

    void list_destroy(list_t *_list);

    size_t list_count(list_t *_list);

    int list_append(list_t *_list, void* _data);

    int list_set_operation(
        list_t *_list, const list_data_op *_op);

#ifdef __cpluplus
}
#endif // __cpluplus

#endif //  _LIST_H
