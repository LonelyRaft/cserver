
#ifndef _NODE_POOL_H
#define _NODE_POOL_H

#include <stddef.h>
#include "node_t.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    int node_pool_init(size_t _size);

    void node_pool_deinit();

    node_t *node_lease();

    int node_release(node_t *_node);

    size_t node_num();

    size_t node_available();

    int dnode_pool_init(size_t _size);

    void dnode_pool_deinit();

    dnode_t *dnode_lease();

    int dnode_release(dnode_t *_node);

    size_t dnode_num();

    size_t dnode_available();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _NODE_POOL_H
