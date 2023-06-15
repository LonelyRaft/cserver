
#include "pool_t.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

typedef struct pool_t
{
    size_t capcity;        // size of this pool
    size_t size;           // size of an element
    size_t count;          // element number in the pool
    void *buffer;          // element memory
    void **pool;           // pool
    pthread_mutex_t *lock; // pool lock
} pool_t;

pool_t *pool_create(size_t _size, size_t _capcity)
{
    pool_t *pool = NULL;
    // check params
    if (_size == 0 || _capcity == 0)
        return pool;
    // alloc memory
    size_t length =
        (_size + sizeof(void*)) * _capcity +
        sizeof(pthread_mutex_t) + sizeof(pool_t);
    char *addr = (char *)malloc(length);
    if (addr == NULL)
        return pool;
    memset(addr, 0, length);
    // realloc memory
    pool = (pool_t *)addr;
    addr = addr + sizeof(pool_t);
    pool->lock = (pthread_mutex_t *)addr;
    addr = addr + sizeof(pthread_mutex_t);
    pool->pool = (void **)addr;
    addr = addr + sizeof(void *) * _capcity;
    pool->buffer = (void *)addr;
    pool->capcity = _capcity;
    pool->size = _size;
    pthread_mutex_init(pool->lock, NULL);
    // init pool
    while (pool->count < pool->capcity)
    {
        pool->pool[pool->count] =
            pool->buffer + pool->count;
        pool->count++;
    }
    return pool;
}

void pool_destroy(pool_t *_pool)
{
    if (_pool == NULL)
        return;
    free(_pool);
    _pool = NULL;
}

size_t pool_capcity(const pool_t *_pool)
{
    if (_pool == NULL)
        return 0;
    // capcity is read only
    return _pool->capcity;
}

size_t ele_size(const pool_t *_pool)
{
    if (_pool == NULL)
        return 0;
    // size is read only
    return _pool->size;
}

size_t ele_count(const pool_t *_pool)
{
    size_t result = 0;
    if (_pool == NULL)
        return result;
    pthread_mutex_lock(_pool->lock);
    result = _pool->count;
    pthread_mutex_unlock(_pool->lock);
    return result;
}

void *ele_lease(pool_t *_pool)
{
    void *result = NULL;
    if (_pool == NULL)
        return result;
    do
    {
        pthread_mutex_lock(_pool->lock);
        if (_pool->count == 0)
            break;
        result = _pool->pool[0];
        _pool->count--;
        _pool->pool[0] =
            _pool->pool[_pool->count];
    } while (0);
    pthread_mutex_unlock(_pool->lock);
    return result;
}

int ele_release(pool_t *_pool, void *_node)
{
    if (_pool == NULL || _node == NULL)
        return 0;
    // buffer and size is readonly
    const void *start = _pool->buffer;
    const void *end =
        (char *)_pool->buffer +
        _pool->capcity * _pool->size;
    if (_node < start || _node >= end)
        return -1;
    size_t length = (char *)_node - (char *)start;
    if (length % _pool->size)
        return -1;
    pthread_mutex_lock(_pool->lock);
    _pool->pool[_pool->count] = _node;
    _pool->count++;
    pthread_mutex_unlock(_pool->lock);
    memset(_node, 0, _pool->size);
    return 0;
}
