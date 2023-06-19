
#include "task_t.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (num < 0) {
        return 0;
    }
    return num;
}
#endif

typedef struct task_vec_t
{
    task_t *tasks;
    size_t capacity;
    size_t count;
    pthread_mutex_t *lock;
} task_vec_t;

task_vec_t *task_vec_create()
{
    size_t num = 2; // proc_num();
    if (num == 0 || num > USHRT_MAX) {
        return NULL;
    }
    size_t length = sizeof(task_vec_t) +
        sizeof(pthread_mutex_t) +
        sizeof(task_t) * num;
    char *addr = (char *)malloc(length);
    if (addr == NULL) {
        return NULL;
    }
    memset(addr, 0, length);
    task_vec_t *vector = (task_vec_t *)addr;
    addr = addr + sizeof(task_vec_t);
    vector->lock = (pthread_mutex_t *)addr;
    addr = addr + sizeof(pthread_mutex_t);
    vector->tasks = (task_t *)addr;
    vector->capacity = num;
    pthread_mutex_init(vector->lock, NULL);
    return vector;
}

void task_vec_destroy(task_vec_t *_vector)
{
    if (_vector != NULL) {
        free(_vector);
        _vector = NULL;
    }
}

const task_t *task_vec_at(
    task_vec_t *_vector, size_t _idx)
{
    if (_vector == NULL ||
        _idx >= _vector->capacity) {
        return NULL;
    }
    return _vector->tasks + _idx;
}

int task_vec_push(task_vec_t *_vector,
    void *(* _entry)(task_t *), void *_arg)
{
    int result = -1;
    if (_vector == NULL || _entry == NULL) {
        return result;
    }
    do {
        pthread_mutex_lock(_vector->lock);
        if (_vector->count == _vector->capacity) {
            result = -2;
            break;
        }
        int idx = 0;
        while (idx < _vector->capacity) {
            task_t *task = _vector->tasks + idx;
            if (task->entry == 0) {
                task->working = 1;
                task->entry = _entry;
                task->arg = _arg;
                result = pthread_create(
                        &task->id, NULL,
                        (void *(*)(void *))task->entry,
                        task);
                if (result == 0) {
                    _vector->count++;
                } else {
                    result = -3;
                }
                break;
            }
            idx++;
        }
    } while (0);
    pthread_mutex_unlock(_vector->lock);
    return result;
}

int task_vec_remove(
    task_vec_t *_vector, size_t _idx)
{
    if (_vector == NULL ||
        _idx >= _vector->capacity) {
        return -1;
    }
    pthread_mutex_lock(_vector->lock);
    task_t *task = _vector->tasks + _idx;
    task->working = 0;
    task->entry = 0;
    _vector->count--;
    pthread_mutex_unlock(_vector->lock);
    return 0;
}

int task_vec_clear(task_vec_t *_vector)
{
    if (_vector == NULL) {
        return -1;
    }
    size_t idx = 0;
    pthread_mutex_lock(_vector->lock);
    while (idx < _vector->capacity) {
        task_t *task = _vector->tasks + idx;
        task->working = 0;
        task->entry = 0;
        _vector->count--;
        idx++;
    }
    pthread_mutex_unlock(_vector->lock);
    return 0;
}

size_t task_vec_count(task_vec_t *_vector)
{
    size_t result = 0;
    if (_vector == NULL) {
        return result;
    }
    pthread_mutex_lock(_vector->lock);
    result = _vector->count;
    pthread_mutex_unlock(_vector->lock);
    return result;
}

size_t task_vec_capacity(task_vec_t *_vector)
{
    size_t result = 0;
    if (_vector == NULL) {
        return result;
    }
    pthread_mutex_lock(_vector->lock);
    result = _vector->capacity;
    pthread_mutex_unlock(_vector->lock);
    return result;
}

int task_vec_full(task_vec_t *_vector)
{
    if (_vector == NULL) {
        return -1;
    }
    pthread_mutex_lock(_vector->lock);
    int result =
        (_vector->count ==
            _vector->capacity);
    pthread_mutex_unlock(_vector->lock);
    return result;
}
