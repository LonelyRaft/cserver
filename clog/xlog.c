
#include "xlog.h"
#include <stdlib.h>
#include <string.h>

clog_t *xlog = NULL;

void xlog_init()
{
    if(xlog != NULL)
        return;
    xlog = clog_read_cfg("./clogcfg.ini");
    if(xlog == NULL)
        xlog = clog_create(1024);
}

void xlog_deinit()
{
    if(xlog == NULL)
        return;
    clog_desrtroy(xlog);
}
