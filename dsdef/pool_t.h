
#ifndef _ELEMENT_POOL_H
#define _ELEMENT_POOL_H

#include <stddef.h>

#ifdef __cpluplus
extern "C"
{
#endif // __cpluplus

    typedef struct pool_t pool_t;

    /********************************************
     *  # create an element pool
     *  @_size: element size
     *  @_capcity: pool capacity
     *  @return: a pool object pointer
     *********************************************/
    pool_t *pool_create(size_t _size, size_t _capcity);

    /********************************************
     *  # destroy the element pool
     *  @_pool: a pool object pointer
     *  @return: no return
     *********************************************/
    void pool_destroy(pool_t *_pool);

    /********************************************
     *  # access the capacity of a pool
     *  @_pool: the pool object pointer to be accessed
     *  @return: the capacity of a pool
     *********************************************/
    size_t pool_capcity(const pool_t *_pool);

    /********************************************
     *  # access the size of an element in the pool
     *  @_pool: the pool object pointer to be accessed
     *  @return: the size of an element
     *********************************************/
    size_t ele_size(const pool_t *_pool);

    /********************************************
     *  # access the number of elements in the pool
     *  @_pool: the pool object pointer
     *  @return: the number of elements
     *********************************************/
    size_t ele_count(const pool_t *_pool);

    /********************************************
     *  # lease an element from the pool
     *  @_pool: the pool object pointer
     *  @return: an element
     *********************************************/
    void *ele_lease(pool_t *_pool);

    /********************************************
     *  # release the element to the pool
     *  @_pool: the pool object pointer
     *  @_node: the element to be release
     *  @return: 0 is success
     *********************************************/
    int ele_release(pool_t *_pool, void *_node);

#ifdef __cpluplus
}
#endif // __cpluplus

#endif // _ELEMENT_POOL_H
