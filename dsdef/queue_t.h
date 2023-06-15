
#ifndef _QUEUE_H
#define _QUEUE_H

#include <stddef.h>
#include "datadef.h"

#ifdef __cpluplus
extern "C"
{
#endif //  __cpluplus

    typedef struct queue_t queue_t;

    queue_t *queue_create();

    void queue_destroy(queue_t *_queue);

    int queue_push(queue_t *_queue, void *_data);

    void *queue_top(const queue_t *_queue);

    void *queue_pop(queue_t *_queue);

    size_t queue_size(const queue_t *_queue);



#ifdef __cpluplus
}
#endif //  __cpluplus

#endif // _QUEUE_H
