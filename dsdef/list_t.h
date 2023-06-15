
#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>
#include "datadef.h"

#ifdef __cpluplus
extern "C"
{
#endif // __cpluplus

    typedef struct list_t list_t;

    list_t *list_create();

    int list_clear(list_t *_list);

    void list_destroy(list_t *_list);

    size_t list_count(const list_t *_list);

    int list_append(list_t *_list, void* _data);

    int list_remove(list_t *_list, void* _data);

    int list_set_operation(
        list_t *_list, const data_op *_op);

    void list_process_data(const list_t *_list);

#ifdef __cpluplus
}
#endif // __cpluplus

#endif //  _LIST_H
