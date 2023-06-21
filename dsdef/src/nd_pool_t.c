
#include "nd_pool_t.h"
#include "pool_t.h"

static pool_t *g_node_pool = NULL;

int node_pool_init(size_t _size)
{
    if (g_node_pool != NULL)
        return 0;
    g_node_pool =
        pool_create(sizeof(node_t), _size);
    if (g_node_pool == NULL)
        return -1;
    return 0;
}

void node_pool_deinit()
{
    if (g_node_pool)
    {
        pool_destroy(g_node_pool);
        g_node_pool = NULL;
    }
}

node_t *node_lease()
{
    return (node_t *)ele_lease(g_node_pool);
}

int node_release(node_t *_node)
{
    return ele_release(g_node_pool, _node);
}

size_t node_num()
{
    return pool_capcity(g_node_pool);
}

size_t node_available()
{
    return ele_count(g_node_pool);
}

static pool_t *g_dnode_pool = NULL;

int dnode_pool_init(size_t _size)
{
    if (g_dnode_pool != NULL)
        return 0;
    g_dnode_pool =
        pool_create(sizeof(dnode_t), _size);
    if (g_dnode_pool == NULL)
        return -1;
    return 0;
}

void dnode_pool_deinit()
{
    if (g_dnode_pool)
    {
        pool_destroy(g_dnode_pool);
        g_dnode_pool = NULL;
    }
}

dnode_t *dnode_lease()
{
    return (dnode_t *)ele_lease(g_dnode_pool);
}

int dnode_release(dnode_t *_node)
{
    return ele_release(g_dnode_pool, _node);
}

size_t dnode_num()
{
    return pool_capcity(g_dnode_pool);
}

size_t dnode_available()
{
    return ele_count(g_dnode_pool);
}
