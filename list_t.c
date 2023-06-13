
#include "list_t.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct list_node_t
{
    void* data;
    struct list_node_t *next;
}list_node_t;

typedef struct list_t {
    const list_data_op *op;
    list_node_t *head;
    list_node_t *tail;
    size_t count;
    pthread_mutex_t lock;
} list_t;

list_t* list_create()
{
    size_t length = sizeof(list_t) + sizeof(list_node_t);
    list_t *list = (list_t*)malloc(length);
    if(list == NULL)
        return NULL;
    memset(list, 0, sizeof(list_t));
    list->head = (list_node_t*)(list + 1);
    pthread_mutex_init(&list->lock, NULL);
    return list;
}

void list_destroy(list_t *_list)
{
    if(_list == NULL)
        return;

    free(_list);
}

