
#include <stdint.h>
#include "co_utils.h"
#include "prf_types.h"
#include "attm_db_128.h"
#include "user_custs2_def.h"

// Service 2 of the custom server 1
static const att_svc_desc128_t custs2_svc1 = DEF_SVC2_UUID_128;

static const uint8_t CUST2_SERVER_TX_UUID_128[ATT_UUID_128_LEN] = DEF_CUST2_SERVER_TX_UUID_128;
static const uint8_t CUST2_SERVER_RX_UUID_128[ATT_UUID_128_LEN] = DEF_CUST2_SERVER_RX_UUID_128;

static struct att_char128_desc custs2_server_rx_char = { ATT_CHAR_PROP_WR_NO_RESP,
                                                         { 0, 0 },
                                                         DEF_CUST2_SERVER_RX_UUID_128 };

static struct att_char128_desc custs2_server_tx_char = { ATT_CHAR_PROP_NTF, { 0, 0 }, DEF_CUST2_SERVER_TX_UUID_128 };

// Attribute specifications
static const uint16_t att_decl_svc = ATT_DECL_PRIMARY_SERVICE;
static const uint16_t att_decl_char = ATT_DECL_CHARACTERISTIC;
static const uint16_t att_desc_cfg = ATT_DESC_CLIENT_CHAR_CFG;
static const uint16_t att_desc_user_desc = ATT_DESC_CHAR_USER_DESCRIPTION;

const uint8_t custs2_services[] = { SVC2_IDX_SVC, CUSTS2_IDX_NB };
const uint8_t custs2_services_size = ARRAY_LEN(custs2_services) - 1;
const uint16_t custs2_att_max_nb = CUSTS2_IDX_NB;

/// Full CUSTS2 Database Description - Used to add attributes into the database
const struct attm_desc_128 custs2_att_db[CUSTS2_IDX_NB] = {

    /*************************
     * Service 2 configuration
     *************************
     */

    // Service 1 Declaration
    [SVC2_IDX_SVC] = { (uint8_t *)&att_decl_svc, ATT_UUID_128_LEN, PERM(RD, ENABLE), sizeof(custs2_svc1),
                       sizeof(custs2_svc1), (uint8_t *)&custs2_svc1 },

    // Server RX Characteristic Declaration
    [CUST2_IDX_SERVER_RX_CHAR] = { (uint8_t *)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                   sizeof(custs2_server_rx_char), sizeof(custs2_server_rx_char),
                                   (uint8_t *)&custs2_server_rx_char },

    // Server RX Characteristic Value
    [CUST2_IDX_SERVER_RX_VAL] = { CUST2_SERVER_RX_UUID_128, ATT_UUID_128_LEN,
                                  PERM(WR, ENABLE) | PERM(WRITE_COMMAND, ENABLE), DEF_CUST2_SERVER_RX_CHAR_LEN, 0,
                                  NULL },

    // Server RX Characteristic User Description
    [CUST2_IDX_SERVER_RX_USER_DESC] = { (uint8_t *)&att_desc_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                        sizeof(CUST2_SERVER_RX_USER_DESC) - 1, sizeof(CUST2_SERVER_RX_USER_DESC) - 1,
                                        (uint8_t *)CUST2_SERVER_RX_USER_DESC },

    // Server TX Characteristic Declaration
    [CUST2_IDX_SERVER_TX_CHAR] = { (uint8_t *)&att_decl_char, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                   sizeof(custs2_server_tx_char), sizeof(custs2_server_tx_char),
                                   (uint8_t *)&custs2_server_tx_char },

    // Server TX Characteristic Value
    [CUST2_IDX_SERVER_TX_VAL] = { CUST2_SERVER_TX_UUID_128, ATT_UUID_128_LEN, PERM(NTF, ENABLE),
                                  DEF_CUST2_SERVER_TX_CHAR_LEN, 0, NULL },

    // Server TX Client Characteristic Configuration Descriptor
    [CUST2_IDX_SERVER_TX_NTF_CFG] = { (uint8_t *)&att_desc_cfg, ATT_UUID_16_LEN,
                                      PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE) |
                                          PERM(WRITE_COMMAND, ENABLE),
                                      sizeof(uint16_t), 0, NULL },

    // Server TX Characteristic User Description
    [CUST2_IDX_SERVER_TX_USER_DESC] = { (uint8_t *)&att_desc_user_desc, ATT_UUID_16_LEN, PERM(RD, ENABLE),
                                        sizeof(CUST2_SERVER_TX_USER_DESC) - 1, sizeof(CUST2_SERVER_TX_USER_DESC) - 1,
                                        (uint8_t *)CUST2_SERVER_TX_USER_DESC },
};
