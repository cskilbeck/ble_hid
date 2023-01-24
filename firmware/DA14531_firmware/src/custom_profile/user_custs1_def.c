//////////////////////////////////////////////////////////////////////
// Custom Server 1 (CUSTS1) profile database definitions.

#include <stdint.h>
#include "co_utils.h"
#include "prf_types.h"
#include "attm_db_128.h"
#include "user_custs1_def.h"

//////////////////////////////////////////////////////////////////////

// Service 1 of the custom server 1
static const att_svc_desc128_t custs1_svc1 = DEF_SVC1_UUID_128;

static const uint8_t SVC1_CTRL_POINT_UUID_128[ATT_UUID_128_LEN] = DEF_SVC1_CTRL_POINT_UUID_128;

// Attribute specifications
static const uint16_t att_decl_svc = ATT_DECL_PRIMARY_SERVICE;
static const uint16_t att_decl_char = ATT_DECL_CHARACTERISTIC;
static const uint16_t att_desc_cfg = ATT_DESC_CLIENT_CHAR_CFG;
static const uint16_t att_desc_user_desc = ATT_DESC_CHAR_USER_DESCRIPTION;

//////////////////////////////////////////////////////////////////////

const uint8_t custs1_services[] = { SVC1_IDX_SVC, CUSTS1_IDX_NB };
const uint8_t custs1_services_size = ARRAY_LEN(custs1_services) - 1;
const uint16_t custs1_att_max_nb = CUSTS1_IDX_NB;

/// Custom Service 1 Database Description - Used to add attributes into the database

// clang-format off
const struct attm_desc_128 custs1_att_db[CUSTS1_IDX_NB] = {

    // Service 1 Declaration
    [SVC1_IDX_SVC] = {
    
        (uint8_t *)&att_decl_svc,
        ATT_UUID_128_LEN,
        PERM(WR, ENABLE),
        sizeof(custs1_svc1),
        sizeof(custs1_svc1),
        (uint8_t *)&custs1_svc1
    },

    // Control Point Characteristic Declaration
    [SVC1_IDX_CONTROL_POINT_CHAR] = {

        (uint8_t *)&att_decl_char,
        ATT_UUID_16_LEN,
        PERM(RD, ENABLE),
        0,
        0,
        NULL
    },

    // Control Point Characteristic Value
    [SVC1_IDX_CONTROL_POINT_VAL] = {
        
        SVC1_CTRL_POINT_UUID_128,
        ATT_UUID_128_LEN,
        PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(WRITE_COMMAND, ENABLE) | PERM(NTF, ENABLE),
        DEF_SVC1_CTRL_POINT_CHAR_LEN,
        0,
        NULL
    },

    // Control Point Client Characteristic Configuration Descriptor
    [SVC1_IDX_CONTROL_POINT_NTF_CFG] = {
        
        (uint8_t *)&att_desc_cfg,
        ATT_UUID_16_LEN,
        PERM(RD, ENABLE) | PERM(WR, ENABLE) | PERM(WRITE_REQ, ENABLE) |PERM(WRITE_COMMAND, ENABLE),
        sizeof(uint16_t),
        0,
        NULL
    },

    // Control Point Characteristic User Description
    [SVC1_IDX_CONTROL_POINT_USER_DESC] = {
        
        (uint8_t *)&att_desc_user_desc,
        ATT_UUID_16_LEN,
        PERM(RD, ENABLE),
        sizeof(DEF_SVC1_CONTROL_POINT_USER_DESC) - 1,
        sizeof(DEF_SVC1_CONTROL_POINT_USER_DESC) - 1,
        (uint8_t *)DEF_SVC1_CONTROL_POINT_USER_DESC
    },
};
// clang-format on
