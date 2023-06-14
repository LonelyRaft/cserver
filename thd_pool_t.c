
#include "thd_pool_t.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
static size_t proc_num()
{
    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}
#endif

#ifdef __linux__
#include <unistd.h>
static size_t proc_num()
{
    int num = sysconf(
        _SC_NPROCESSORS_ONLN);
    if (num < 0)
        return 0;
    return num;
}
#endif

typedef struct thd_pool_t
{
    thd_t *thds;
    size_t size;
    size_t count;
    pthread_mutex_t lock;
} thd_pool_t;

thd_pool_t *thd_pool_create()
{
    size_t num = proc_num();
    if (num == 0 || num > USHRT_MAX)
        return NULL;
    size_t length =
        sizeof(thd_t) * num;
    length += sizeof(thd_pool_t);
    thd_pool_t *pool =
        (thd_pool_t *)malloc(length);
    if (pool == NULL)
        return NULL;
    memset(pool, 0, length);
    pool->thds =
        (thd_t *)(pool + 1);
    pool->size = num;
    pthread_mutex_init(
        &pool->lock, NULL);
    return pool;
}

void thd_pool_destroy(
    thd_pool_t *_thd_pool)
{
    if (_thd_pool == NULL)
        return;
    free(_thd_pool);
}

int thd_pool_add(
    thd_pool_t *_thd_pool,
    const thd_t _thd)
{
    int result = -2;
    if (_thd_pool == NULL || _thd.id == 0 ||
        _thd.entry == NULL || _thd.arg == NULL)
        return -1;
    do
    {
        pthread_mutex_lock(&_thd_pool->lock);
        if (_thd_pool->count == _thd_pool->size)
            break;
        int idx = 0;
        while (idx < _thd_pool->size)
        {
            if (_thd_pool->thds[idx].id == 0)
            {
                _thd_pool->thds[idx] = _thd;
                _thd_pool->count++;
                result = 0;
                break;
            }
            idx++;
        }
    } while (0);
    pthread_mutex_unlock(&_thd_pool->lock);
    return result;
}

int thd_pool_remove(
    thd_pool_t *_thd_pool,
    thd_t *_thd)
{
    if (_thd_pool == NULL || _thd == NULL)
        return -1;
    do
    {
        pthread_mutex_lock(&_thd_pool->lock);
        if (_thd_pool->count == 0)
            break;
        int idx = 0;
        while (idx < _thd_pool->size)
        {
            if (_thd_pool->thds[idx].id == _thd->id)
            {
                *_thd = _thd_pool->thds[idx];
                _thd_pool->count--;
                break;
            }
            idx++;
        }
    } while (0);
    pthread_mutex_unlock(&_thd_pool->lock);
    return 0;
}

int thd_pool_find(
    thd_pool_t *_thd_pool,
    thd_t *_thd)
{
    int result = -2;
    if (_thd_pool == NULL || _thd == NULL)
        return -1;
    do
    {
        pthread_mutex_lock(&_thd_pool->lock);
        if (_thd_pool->count == 0)
            break;
        int idx = 0;
        while (idx < _thd_pool->size)
        {
            if (_thd_pool->thds[idx].id == _thd->id)
            {
                *_thd = _thd_pool->thds[idx];
                break;
            }
            idx++;
        }
        if (idx == _thd_pool->size)
            result = -3;
    } while (0);
    pthread_mutex_unlock(&_thd_pool->lock);
    return result;
}

int thd_pool_get(
    thd_pool_t *_thd_pool,
    size_t _idx, thd_t *_thd)
{
    int result = -2;
    if (_thd_pool == NULL || _thd == NULL ||
        _idx >= _thd_pool->size)
        return -1;
    pthread_mutex_lock(&_thd_pool->lock);
    if (_thd_pool->thds[_idx].id != 0)
    {
        *_thd = _thd_pool->thds[_idx];
        result = 0;
    }
    pthread_mutex_unlock(&_thd_pool->lock);
    return result;
}

size_t thd_pool_count(
    thd_pool_t *_thd_pool)
{
    size_t thdcnt = 0;
    if (_thd_pool == NULL)
        return thdcnt;
    pthread_mutex_lock(&_thd_pool->lock);
    thdcnt = _thd_pool->count;
    pthread_mutex_unlock(&_thd_pool->lock);
    return thdcnt;
}

size_t thd_pool_size(
    thd_pool_t *_thd_pool)
{
    size_t thdsz = 0;
    if (_thd_pool == NULL)
        return thdsz;
    pthread_mutex_lock(&_thd_pool->lock);
    thdsz = _thd_pool->size;
    pthread_mutex_unlock(&_thd_pool->lock);
    return thdsz;
}

int thd_pool_full(
    thd_pool_t *_thd_pool)
{
    int result = 0;
    if (_thd_pool == NULL)
        return 1;
    pthread_mutex_lock(&_thd_pool->lock);
    if (_thd_pool->count == _thd_pool->size)
        result = 1;
    pthread_mutex_unlock(&_thd_pool->lock);
    return result;
}

int thd_valid(const thd_t *_thd)
{
    if (_thd == NULL)
        return 0;
    if (_thd->id == 0 ||
        _thd->entry == NULL ||
        _thd->arg == NULL)
        return 0;
    return 1;
}
