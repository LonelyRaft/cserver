
#include "pool_t.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

typedef struct pool_t
{
    size_t capcity; // size of this pool
    size_t size; // size of an element
    size_t count; // element number in the pool
    void *buffer; // element memory
    void **pool; // pool stack
    unsigned char *flags; // lease flag
    pthread_mutex_t *lock; // pool lock
} pool_t;

static int pool_get_ele_flag(pool_t *_pool, size_t _idx)
{
    if (_pool == NULL ||
        _idx >= _pool->capcity) {
        return -1;
    }
    size_t byte_idx = (_idx >> 3); // _idx / 8
    size_t bit_idx = (_idx & 0x07); // _idx % 8
    unsigned char bit_mask = (0x01 << bit_idx);
    unsigned char flags = _pool->flags[byte_idx];
    flags &= bit_mask;
    return flags;
}

static int pool_set_ele_flag(pool_t *_pool, size_t _idx)
{
    if (_pool == NULL ||
        _idx >= _pool->capcity) {
        return -1;
    }
    size_t byte_idx = (_idx >> 3); // _idx / 8
    size_t bit_idx = (_idx & 0x07); // _idx % 8
    unsigned char bit_mask = (0x01 << bit_idx);
    unsigned char flags = _pool->flags[byte_idx];
    flags ^= bit_mask;
    _pool->flags[byte_idx] = flags;
    return flags;
}

pool_t *pool_create(size_t _size, size_t _capcity)
{
    pool_t *pool = NULL;
    // check params
    if (_size == 0 || _capcity == 0) {
        return pool;
    }
    // alloc memory
    size_t length =
        _size * _capcity + // element buffer
        sizeof(void *) * _capcity + // pool stack
        ((_capcity >> 3) + 1) + // flags
        sizeof(pthread_mutex_t) + sizeof(pool_t);
    char *addr = (char *)malloc(length);
    if (addr == NULL) {
        return pool;
    }
    memset(addr, 0, length);
    // realloc memory
    pool = (pool_t *)addr;
    addr = addr + sizeof(pool_t);
    pool->lock = (pthread_mutex_t *)addr;
    addr = addr + sizeof(pthread_mutex_t);
    pool->buffer = (void *)addr;
    addr = addr + _size * _capcity;
    pool->pool = (void **)addr;
    addr = addr + sizeof(void *) * _capcity;
    pool->flags = (unsigned char *)addr;
    pool->capcity = _capcity;
    pool->size = _size;
    pthread_mutex_init(pool->lock, NULL);
    // init pool
    size_t idx = 0;
    while (pool->count < pool->capcity) {
        pool->pool[pool->count] =
            (char*)pool->buffer + idx;
        idx += pool->size;
        pool->count++;
    }
    return pool;
}

void pool_destroy(pool_t *_pool)
{
    if (_pool == NULL) {
        return;
    }
    free(_pool);
    _pool = NULL;
}

size_t pool_capcity(const pool_t *_pool)
{
    if (_pool == NULL) {
        return 0;
    }
    // capcity is read only
    return _pool->capcity;
}

size_t ele_size(const pool_t *_pool)
{
    if (_pool == NULL) {
        return 0;
    }
    // size is read only
    return _pool->size;
}

size_t ele_count(const pool_t *_pool)
{
    size_t result = 0;
    if (_pool == NULL) {
        return result;
    }
    pthread_mutex_lock(_pool->lock);
    result = _pool->count;
    pthread_mutex_unlock(_pool->lock);
    return result;
}

void *ele_lease(pool_t *_pool)
{
    void *result = NULL;
    if (_pool == NULL) {
        return result;
    }
    do {
        pthread_mutex_lock(_pool->lock);
        if (_pool->count == 0) {
            break;
        }
        _pool->count--;
        result = _pool->pool[_pool->count];
        size_t idx = (char *)result -
            (char *)_pool->buffer;
        idx /= _pool->size;
        pool_set_ele_flag(_pool, idx);
    } while (0);
    pthread_mutex_unlock(_pool->lock);
    return result;
}

int ele_release(pool_t *_pool, void *_ele)
{
    if (_pool == NULL || _ele == NULL) {
        return 0;
    }
    // buffer and size is readonly
    const void *start = _pool->buffer;
    const void *end =
        (char *)_pool->buffer +
        _pool->capcity * _pool->size;
    if (_ele < start || _ele >= end) {
        return -1;
    }
    size_t idx = (char *)_ele - (char *)start;
    if (idx % _pool->size) {
        return -2;
    }
    idx /= _pool->size;
    if (!pool_get_ele_flag(_pool, idx)) {
        return -3;
    }
    pthread_mutex_lock(_pool->lock);
    _pool->pool[_pool->count] = _ele;
    _pool->count++;
    pool_set_ele_flag(_pool, idx);
    pthread_mutex_unlock(_pool->lock);
    return 0;
}
