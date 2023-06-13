
#ifndef _THREAD_LIST_H
#define _THREAD_LIST_H

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef struct thd_pool_t thd_pool_t;

    typedef struct thd_t
    {
        pthread_t id;
        void *(*entry)(void *);
        void *arg;
    } thd_t;

    thd_pool_t *thd_pool_create();

    void thd_pool_destroy(
        thd_pool_t *_thd_pool);

    int thd_pool_find(
        thd_pool_t *_thd_pool, thd_t *_thd);

    int thd_pool_get(
        thd_pool_t *_thd_pool,
        int _idx, thd_t *_thd);

    int thd_pool_add(
        thd_pool_t *_thd_pool,
        const thd_t _thd);

    int thd_pool_remove(
        thd_pool_t *_thd_pool,
        thd_t *_thd);

    size_t thd_pool_count(
        thd_pool_t *_thd_pool);

    size_t thd_pool_size(
        thd_pool_t *_thd_pool);

    int thd_pool_full(
        thd_pool_t *_thd_pool);

    int thd_valid(const thd_t *_thd);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _THREAD_LIST_H
