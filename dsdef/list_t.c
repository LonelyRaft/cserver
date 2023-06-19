
#include "list_t.h"
#include "nd_pool_t.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct list_t
{
    const data_op *op;
    node_t *head;
    node_t *tail;
    node_t *it;
    size_t count;
    pthread_mutex_t *lock;
} list_t;

list_t *list_create(const data_op *_op)
{
    size_t length =
        sizeof(list_t) + sizeof(node_t) +
        sizeof(pthread_mutex_t);
    list_t *list = (list_t *)malloc(length);
    if (list == NULL) {
        return NULL;
    }
    memset(list, 0, length);
    list->op = _op;
    list->head = (node_t *)(list + 1);
    list->tail = list->head;
    list->lock = (pthread_mutex_t *)(list->head + 1);
    pthread_mutex_init(list->lock, NULL);
    return list;
}

void list_destroy(list_t *_list)
{
    if (_list == NULL) {
        return;
    }
    list_clear(_list);
    free(_list);
    _list = NULL;
}

int list_clear(list_t *_list)
{
    int result = -1;
    if (_list == NULL) {
        return -1;
    }
    if (_list->op == NULL ||
        _list->op->destroy == NULL) {
        return -2;
    }
    pthread_mutex_lock(_list->lock);
    node_t *curr = _list->head;
    while (curr->next) {
        node_t *todel = curr->next;
        curr->next = todel->next;
        if (todel->data != NULL) {
            _list->op->destroy(todel->data);
        }
        todel->data = NULL;
        todel->next = NULL;
        node_release(todel);
        _list->count--;
    }
    _list->tail = _list->head;
    result = _list->count;
    pthread_mutex_unlock(_list->lock);
    return result;
}

size_t list_count(const list_t *_list)
{
    size_t result = 0;
    if (_list == NULL) {
        return result;
    }
    pthread_mutex_lock(_list->lock);
    result = _list->count;
    pthread_mutex_unlock(_list->lock);
    return result;
}

int list_push(list_t *_list, void *_data)
{
    if (_list == NULL || _data == NULL) {
        return -1;
    }
    if (_list->op == NULL ||
        _list->op->create == NULL) {
        return -2;
    }
    node_t *node = node_lease();
    if (node == NULL) {
        return -3;
    }
    node->next = 0;
    node->data = _list->op->create(_data);
    if (node->data == NULL) {
        node_release(node);
        return -4;
    }
    pthread_mutex_lock(_list->lock);
    _list->tail->next = node;
    _list->tail = node;
    _list->count++;
    pthread_mutex_unlock(_list->lock);
    return 0;
}

void *list_pop(list_t *_list)
{
    void *data = NULL;
    if (_list == NULL) {
        return data;
    }
    do {
        pthread_mutex_lock(_list->lock);
        node_t *top = _list->head->next;
        if (top == NULL) {
            break;
        }
        data = top->data;
        _list->head->next = top->next;
        node_release(top);
        _list->count--;
    } while (0);
    pthread_mutex_unlock(_list->lock);
    return data;
}

void *list_top(const list_t *_list)
{
    if (_list == NULL) {
        return NULL;
    }
    pthread_mutex_lock(_list->lock);
    node_t *top = _list->head->next;
    pthread_mutex_unlock(_list->lock);
    if (top == NULL) {
        return NULL;
    }
    return top->data;
}

int list_remove(list_t *_list, void *_data)
{
    if (_list == NULL || _data == NULL) {
        return 0;
    }
    if (_list->op == NULL ||
        _list->op->cmp == NULL ||
        _list->op->destroy == NULL) {
        return -2;
    }
    node_t *node = _list->head;
    pthread_mutex_lock(_list->lock);
    while (node->next) {
        void *nd_data = node->next->data;
        if (_data != nd_data &&
            _list->op->cmp(_data, nd_data)) {
            node = node->next;
            continue;
        }
        node_t *todel = node->next;
        if (_list->it == todel) {
            _list->it = node;
        }
        if (_list->tail == todel) {
            _list->tail = node;
        }
        node->next = todel->next;
        todel->next = NULL;
        todel->data = NULL;
        if (nd_data != NULL) {
            _list->op->destroy(nd_data);
        }
        node_release(todel);
        _list->count--;
        break;
    }
    pthread_mutex_unlock(_list->lock);
    return 0;
}

void *list_begin(list_t *_list)
{
    void *item = NULL;
    if (_list == NULL) {
        return item;
    }
    pthread_mutex_lock(_list->lock);
    _list->it = _list->head->next;
    if (_list->it != NULL) {
        item = _list->it->data;
    }
    pthread_mutex_unlock(_list->lock);
    return item;
}

void *list_next(list_t *_list)
{
    void *item = NULL;
    if (_list == NULL) {
        return item;
    }
    do {
        pthread_mutex_lock(_list->lock);
        if (_list->it == NULL) {
            break;
        }
        _list->it = _list->it->next;
        if (_list->it == NULL) {
            break;
        }
        item = _list->it->data;
    } while (0);
    pthread_mutex_unlock(_list->lock);
    return item;
}
