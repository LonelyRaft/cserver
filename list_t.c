
#include "list_t.h"
#include "nd_pool_t.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct list_t
{
    const list_data_op *op;
    node_t *head;
    node_t *tail;
    size_t count;
    pthread_mutex_t lock;
} list_t;

list_t *list_create()
{
    size_t length = sizeof(list_t) + sizeof(node_t);
    list_t *list = (list_t *)malloc(length);
    if (list == NULL)
        return NULL;
    memset(list, 0, sizeof(list_t));
    list->head = (node_t *)(list + 1);
    pthread_mutex_init(&list->lock, NULL);
    return list;
}

void list_destroy(list_t *_list)
{
    if (_list == NULL)
        return;
    list_clear(_list);
    free(_list);
    _list = NULL;
}

int list_clear(list_t *_list)
{
    int result = -1;
    if (_list == NULL)
        return result;
    pthread_mutex_lock(&_list->lock);
    node_t *curr = _list->head;
    while (curr->next)
    {
        node_t *todel = curr->next;
        curr->next = todel->next;
        // free(todel->data);
        node_release(todel);
        _list->count--;
    }
    _list->tail = _list->head;
    result = _list->count;
    pthread_mutex_unlock(&_list->lock);
    return result;
}

int list_set_operation(
    list_t *_list, const list_data_op *_op)
{
    if (_list == NULL || _op == NULL)
        return -1;
    return 0;
}
