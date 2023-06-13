
#include "queue_t.h"
#include "nd_pool_t.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct queue_t
{
    node_t *head;
    node_t *tail;
    size_t count;
    pthread_mutex_t lock;
} queue_t;

queue_t *queue_create()
{
    size_t length = sizeof(queue_t) + sizeof(node_t);
    queue_t *queue = (queue_t *)malloc(length);
    if (queue == NULL)
        return NULL;
    memset(queue, 0, length);
    queue->head = (node_t *)(queue + 1);
    queue->tail = queue->head;
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void queue_destroy(
    queue_t *_queue)
{
    if (_queue == NULL)
        return;
    queue_clear(_queue);
    free(_queue);
    _queue = NULL;
}

int queue_clear(queue_t *_queue)
{
    if (_queue == NULL)
        return -1;
    pthread_mutex_lock(&_queue->lock);
    node_t *curr = _queue->head;
    while (curr->next)
    {
        node_t *todel = curr->next;
        curr->next = todel->next;
        // free(todel->data);
        node_release(todel);
        _queue->count--;
    }
    _queue->tail = _queue->head;
    size_t ndcnt = _queue->count;
    pthread_mutex_unlock(&_queue->lock);
    return ndcnt;
}

int queue_push(queue_t *_queue, void *_data)
{
    if (_queue == NULL || _data == NULL)
        return -1;
    node_t *node = node_lease();
    if (node == NULL)
        return -2;
    node->data = _data;
    node->next = 0;
    pthread_mutex_lock(&_queue->lock);
    _queue->tail->next = node;
    _queue->tail = node;
    _queue->count++;
    pthread_mutex_unlock(&_queue->lock);
    return 0;
}

void *queue_top(queue_t *_queue)
{
    if (_queue == NULL)
        return NULL;
    pthread_mutex_lock(&_queue->lock);
    node_t *top = _queue->head->next;
    pthread_mutex_unlock(&_queue->lock);
    if (top == NULL)
        return NULL;
    return top->data;
}

void *queue_pop(queue_t *_queue)
{
    void *data = NULL;
    if (_queue == NULL)
        return data;
    do
    {
        pthread_mutex_lock(&_queue->lock);
        node_t *top = _queue->head->next;
        if (top == NULL)
            break;
        data = top->data;
        _queue->head->next = top->next;
        node_release(top);
        _queue->count--;
    } while (0);
    pthread_mutex_unlock(&_queue->lock);
    return data;
}

size_t queue_size(queue_t *_queue)
{
    size_t size = 0;
    if (_queue == NULL)
        return size;
    pthread_mutex_lock(&_queue->lock);
    size = _queue->count;
    pthread_mutex_unlock(&_queue->lock);
    return size;
}
