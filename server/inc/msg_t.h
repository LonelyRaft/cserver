
#ifndef _DATA_MESSGAE_H
#define _DATA_MESSGAE_H

#include "sktop.h"

#ifdef __cplusplus
extern "C" {
#endif //  __cplusplus

#define SKT_TYPE_INV 0x00
#define SKT_TYPE_INTF 0x01
#define SKT_TYPE_DATA 0x02
#define SKT_TYPE_LIST 0x03

typedef struct loc_msg_t
{
    unsigned char type;
    skt_t id;
}loc_msg_t;

#ifdef __cplusplus
}
#endif //  __cplusplus

#endif //_DATA_MESSGAE_H
