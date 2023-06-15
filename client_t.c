
#include "client_t.h"

static client_t* client_create(client_t *_client)
{
    return NULL;
}

static int client_destroy(client_t *_client)
{
    return 0;
}

static int client_copy(client_t *_dest, client_t *_src)
{
    return 0;
}

static int client_cmp(const client_t *_a, const client_t *_b)
{
    return 0;
}

static int client_process(client_t *_data)
{
    return 0;
}

static const list_data_op op1 = {
    DATA_COPY copy,
    DATA_EQUAL equal,
    DATA_PROCESS process,
};

int apply_operation(list_t *_list)
{
    return list_set_operation(_list, &op1);
}
