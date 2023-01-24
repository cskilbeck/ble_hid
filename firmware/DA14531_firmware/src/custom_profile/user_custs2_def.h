//////////////////////////////////////////////////////////////////////
// CUSTOM SERVICE 2 is HID OVER GATT (HOG)

#pragma once

#include "attm_db_128.h"

//////////////////////////////////////////////////////////////////////
// Service 1 of the custom server 2

#define DEF_SVC2_UUID_128                                                                              \
    {                                                                                                  \
        0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07 \
    }

#define DEF_CUST2_SERVER_TX_UUID_128                                                                   \
    {                                                                                                  \
        0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07 \
    }

#define DEF_CUST2_SERVER_RX_UUID_128                                                                   \
    {                                                                                                  \
        0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07 \
    }

// length = MTU - 3, change it when increasing MTU or use DLE
#define DEF_CUST2_SERVER_TX_CHAR_LEN (247 - 3)
#define DEF_CUST2_SERVER_RX_CHAR_LEN (247 - 3)

#define CUST2_SERVER_TX_USER_DESC "Server TX Data"
#define CUST2_SERVER_RX_USER_DESC "Server RX Data"

//////////////////////////////////////////////////////////////////////
// Custom2 Service Data Base Characteristic enum

enum
{
    // Custom Service 1
    SVC2_IDX_SVC = 0,

    CUST2_IDX_SERVER_RX_CHAR,
    CUST2_IDX_SERVER_RX_VAL,
    CUST2_IDX_SERVER_RX_USER_DESC,

    CUST2_IDX_SERVER_TX_CHAR,
    CUST2_IDX_SERVER_TX_VAL,
    CUST2_IDX_SERVER_TX_NTF_CFG,
    CUST2_IDX_SERVER_TX_USER_DESC,

    CUSTS2_IDX_NB
};
