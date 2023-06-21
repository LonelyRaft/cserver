
#include "xlog.h"
#include <stdlib.h>
#include <string.h>

clog_t *xlog = NULL;

void xlog_init()
{
    if (xlog != NULL) {
        return;
    }
#ifdef __linux__
    xlog = clog_read_cfg("/etc/clogcfg.ini");
#endif // __linux__
#ifdef _WIN32
    xlog = clog_read_cfg("./clogcfg.ini");
#endif // _WIN32
    if (xlog == NULL) {
        xlog = clog_create(1024);
    }
}

void xlog_deinit()
{
    if (xlog == NULL) {
        return;
    }
    clog_desrtroy(xlog);
}
