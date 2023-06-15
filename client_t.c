
#include "client_t.h"

static client_t *client_create(client_t *_client)
{
    return NULL;
}

static void client_destroy(client_t *_client)
{
    return;
}

static void client_copy(client_t *_dest, const client_t *_src)
{
    return;
}

static int client_cmp(const client_t *_a, const client_t *_b)
{
    return 0;
}

list_t* client_list_create()
{
    static const data_op client_op = {
        DATA_CREATE client_create,
        DATA_DESTROY client_destroy,
        DATA_COPY client_copy,
        DATA_COMPARE client_cmp,
    };
    return list_create(&client_op);
}

int client_run(client_t* _client)
{
    return 0;
}
