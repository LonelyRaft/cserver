
#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>
#include "datatype_t.h"

#ifdef __cpluplus
extern "C"
{
#endif // __cpluplus

    typedef struct list_t list_t;

    list_t *list_create(const data_op *_op);

    int list_clear(list_t *_list);

    void list_destroy(list_t *_list);

    size_t list_count(const list_t *_list);

    int list_push(list_t *_list, void *_data);

    void *list_top(const list_t *_list);

    void *list_pop(list_t *_list);

    int list_remove(list_t *_list, void *_data);

    void *list_begin(list_t *_list);

    void *list_next(list_t *_list);

#ifdef __cpluplus
}
#endif // __cpluplus

#endif //  _LIST_H
