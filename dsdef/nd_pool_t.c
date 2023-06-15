
#include "nd_pool_t.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_POOLSZ                    \
    ((UINT_MAX - sizeof(nd_pool_t)) / \
     (sizeof(node_t *) + sizeof(node_t)))

#define POOL_SIZE(size)            \
    ((sizeof(node_t *) * (size)) + \
     (sizeof(node_t) * (size)) +   \
     sizeof(nd_pool_t))

typedef struct nd_pool_t
{
    size_t size;
    size_t idx;
    node_t *buffer;
    node_t **pool;
    pthread_mutex_t lock;
} nd_pool_t;

static nd_pool_t *nd_pool_create(size_t _size)
{
    if (_size == 0 || _size >= MAX_POOLSZ)
        return NULL;
    size_t length = POOL_SIZE(_size);
    char *addr = (char *)malloc(length);
    if (addr == NULL)
        return NULL;
    memset(addr, 0, length);
    nd_pool_t *pool =
        (nd_pool_t *)addr;
    addr = addr + sizeof(nd_pool_t);
    pool->size = _size;
    pool->buffer = (node_t *)addr;
    addr = addr + sizeof(node_t) * _size;
    pool->pool = (node_t **)addr;
    pthread_mutex_init(&pool->lock, NULL);
    while (pool->idx < pool->size)
    {
        pool->pool[pool->idx] =
            pool->buffer + pool->idx;
        pool->idx++;
    }
    return pool;
}

static void nd_pool_destroy(nd_pool_t *_pool)
{
    if (_pool == NULL)
        return;
    free(_pool);
}

static size_t nd_pool_size(nd_pool_t *_pool)
{
    if (_pool == NULL)
        return 0;
    // size is read only
    return _pool->size;
}

static size_t nd_pool_count(nd_pool_t *_pool)
{
    int result = 0;
    if (_pool == NULL)
        return result;
    pthread_mutex_lock(&_pool->lock);
    result = _pool->idx;
    pthread_mutex_unlock(&_pool->lock);
    return result;
}

static node_t *nd_pool_lease(nd_pool_t *_pool)
{
    node_t *result = NULL;
    if (_pool == NULL)
        return result;
    do
    {
        pthread_mutex_lock(&_pool->lock);
        if (_pool->idx == 0)
            break;
        result = _pool->pool[0];
        _pool->idx--;
        _pool->pool[0] =
            _pool->pool[_pool->idx];
    } while (0);
    pthread_mutex_unlock(&_pool->lock);
    return result;
}

static int nd_pool_release(
    nd_pool_t *_pool, node_t *_node)
{
    if (_pool == NULL || _node == NULL)
        return 0;
    // buffer and size is readonly
    if (_node < _pool->buffer ||
        _node >= (_pool->buffer + _pool->size))
        return -1;
    size_t length = (char *)_node -
                    (char *)_pool->buffer;
    if (length % sizeof(node_t))
        return -1;
    pthread_mutex_lock(&_pool->lock);
    _pool->pool[_pool->idx] = _node;
    _pool->idx++;
    pthread_mutex_unlock(&_pool->lock);
    memset(_node, 0, sizeof(node_t));
    return 0;
}

static nd_pool_t *nd_pool = NULL;

int node_pool_init(size_t _size)
{
    if (nd_pool != NULL)
        return 0;
    nd_pool = nd_pool_create(_size);
    if (nd_pool == NULL)
        return -1;
    return 0;
}

void node_pool_deinit()
{
    nd_pool_destroy(nd_pool);
    nd_pool = NULL;
}

node_t *node_lease()
{
    return nd_pool_lease(nd_pool);
}

int node_release(node_t *_node)
{
    return nd_pool_release(nd_pool, _node);
}

size_t node_num()
{
    return nd_pool_size(nd_pool);
}

size_t node_available()
{
    return nd_pool_count(nd_pool);
}
