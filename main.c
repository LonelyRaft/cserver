#include <stdio.h>
#include <stdlib.h>
// #include "nd_pool_t.h"

int main()
{
    //    node_pool_init(3);

    void *data1 = malloc(16);

    void *data2 = malloc(16);

    free(data1);

    free(data2);

    //    node_pool_deinit();
    return 0;
}
