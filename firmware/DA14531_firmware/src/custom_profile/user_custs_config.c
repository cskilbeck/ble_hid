//////////////////////////////////////////////////////////////////////

#include "app_prf_types.h"
#include "app_customs.h"
#include "user_custs1_def.h"
#include "user_custs2_def.h"

//////////////////////////////////////////////////////////////////////

#if (BLE_CUSTOM1_SERVER)
extern const struct attm_desc_128 custs1_att_db[CUSTS1_IDX_NB];
#endif

#if (BLE_CUSTOM2_SERVER)
extern const struct attm_desc_128 custs2_att_db[CUSTS2_IDX_NB];
#endif

/// Custom1/2 server function callback table
const struct cust_prf_func_callbacks cust_prf_funcs[] =
{
#if (BLE_CUSTOM1_SERVER)
    {   TASK_ID_CUSTS1,
        custs1_att_db,
        CUSTS1_IDX_NB,
        app_custs1_create_db, NULL,
        NULL, NULL,
    },
#endif
#if (BLE_CUSTOM2_SERVER)
    {   TASK_ID_CUSTS2,
        custs2_att_db,
        CUSTS2_IDX_NB,
        app_custs2_create_db, NULL,
        NULL, NULL,
    },
#endif
    {TASK_ID_INVALID, NULL, 0, NULL, NULL, NULL, NULL},   // DO NOT MOVE. Must always be last
};
