
#ifndef XLOG_H
#define XLOG_H

#include "clog.h"

extern clog_t *xlog;
#define xlogError(fmt, ...) clog_error(xlog, fmt, ##__VA_ARGS__)
#define xlogWarn(fmt, ...) clog_warn(xlog, fmt, ##__VA_ARGS__)
#define xlogInfo(fmt, ...) clog_info(xlog, fmt, ##__VA_ARGS__)
#define xlogDebug(fmt, ...) clog_debug(xlog, fmt, ##__VA_ARGS__)
#define xlogStatus(fmt, ...) clog_status(xlog, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void xlog_init();

void xlog_deinit();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // XLOG_H

