
#include "client_t.h"

static int copy(client_t *_dest, client_t *_src)
{
    return 0;
}

static int equal(const client_t *_a, const client_t *_b)
{
    return 0;
}

static int process(client_t *_data)
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
