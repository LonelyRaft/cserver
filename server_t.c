
#include "server_t.h"
#include <pthread.h>
#include "thd_pool_t.h"
//#include "list_t.h"
#include "array_t.h"

typedef struct server_t
{
    thd_pool_t *pool;
    array_t *array;
    pthread_mutex_t lock;
}server_t;

