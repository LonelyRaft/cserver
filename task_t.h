
#ifndef _THREAD_LIST_H
#define _THREAD_LIST_H

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct task_vec_t task_vec_t;

typedef struct task_t
{
    pthread_t id;
    void *(*entry)(struct task_t *);
    void *arg;
    unsigned char working;
} task_t;

task_vec_t *task_vec_create(int _capcity);

void task_vec_destroy(task_vec_t *_vector);

const task_t *task_vec_at(
    task_vec_t *_vector, size_t _idx);

int task_vec_push(task_vec_t *_vector,
    void *(* _entry)(task_t *), void *_arg);

int task_vec_remove(
    task_vec_t *_vector, size_t _idx);

int task_vec_clear(task_vec_t *_vector);

size_t task_vec_count(task_vec_t *_vector);

size_t task_vec_capacity(task_vec_t *_vector);

int task_vec_full(task_vec_t *_vector);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _THREAD_LIST_H
