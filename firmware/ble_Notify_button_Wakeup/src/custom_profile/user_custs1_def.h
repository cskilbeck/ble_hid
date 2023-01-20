//////////////////////////////////////////////////////////////////////

#ifndef _USER_CUSTS1_DEF_H_
#define _USER_CUSTS1_DEF_H_

//////////////////////////////////////////////////////////////////////

#include "attm_db_128.h"

//////////////////////////////////////////////////////////////////////
// Service 1 of the custom server 1

#define DEF_SVC1_UUID_128                {0x59, 0x5a, 0x08, 0xe4, 0x86, 0x2a, 0x9e, 0x8f, 0xe9, 0x11, 0xbc, 0x7c, 0x98, 0x43, 0x42, 0x18}

#define DEF_SVC1_CTRL_POINT_UUID_128     {0x20, 0xEE, 0x8D, 0x0C, 0xE1, 0xF0, 0x4A, 0x0C, 0xB3, 0x25, 0xDC, 0x53, 0x6A, 0x68, 0x86, 0x2D}

#define DEF_SVC1_CTRL_POINT_CHAR_LEN     15

#define DEF_SVC1_CONTROL_POINT_USER_DESC     "Control Point"

//////////////////////////////////////////////////////////////////////
// Custom1 Service Data Base Characteristic enum
enum
{
    // Custom Service 1
    SVC1_IDX_SVC = 0,

    SVC1_IDX_CONTROL_POINT_CHAR,
    SVC1_IDX_CONTROL_POINT_VAL,
    SVC1_IDX_CONTROL_POINT_NTF_CFG,
    SVC1_IDX_CONTROL_POINT_USER_DESC,

    CUSTS1_IDX_NB
};

#endif // _USER_CUSTS1_DEF_H_
