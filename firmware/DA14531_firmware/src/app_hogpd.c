
#include "rwip_config.h"    // SW configuration

#if(BLE_HID_DEVICE)

#include "app_hogpd.h"
#include "prf_utils.h"
#include "app_prf_perm_types.h"
#include "arch_console.h"

const hogpd_reports_t hogpd_reports[HID_NUM_OF_REPORTS] = {

#if HID_WITH_KEYBOARD
    [HID_KEYBOARD_REPORT_IDX] = { .id = HID_KEYBOARD_REPORT_ID,
                                  .size = HID_KEYBOARD_REPORT_SIZE,
                                  .cfg = HOGPD_CFG_REPORT_IN | HOGPD_REPORT_NTF_CFG_MASK | HOGPD_CFG_REPORT_WR,
                                  .read_callback = NULL,
                                  .write_callback = NULL },

#endif

#if HID_WITH_MOUSE
    [HID_MOUSE_REPORT_IDX] = { .id = HID_MOUSE_REPORT_ID,
                               .size = HID_MOUSE_REPORT_SIZE,
                               .cfg = HOGPD_CFG_REPORT_IN | HOGPD_REPORT_NTF_CFG_MASK | HOGPD_CFG_REPORT_WR,
                               .read_callback = NULL,
                               .write_callback = NULL },
#endif

    [HID_CONSUMER_REPORT_IDX] = { .id = HID_CONSUMER_REPORT_ID,
                                  .size = HID_CONSUMER_REPORT_SIZE,
                                  .cfg = HOGPD_CFG_REPORT_IN | HOGPD_REPORT_NTF_CFG_MASK | HOGPD_CFG_REPORT_WR,
                                  .read_callback = NULL,
                                  .write_callback = NULL }
};

/**
 ****************************************************************************************
 * Note on RemoteWakeup mode: Remote Host may not handle properly remote wakeup when the
 * inactivity timeout is on. Some Hosts do not expect to receive LL_TERMINATE_IND from
 * Wakeup capable devices while they are sleeping.
 ****************************************************************************************
 */
const hogpd_params_t hogpd_params = {

    .boot_protocol_mode = false,
    .batt_external_report = false,
    .remote_wakeup = false,

/**
 ****************************************************************************************
 * Set normally connectable mode
 ****************************************************************************************
 */
#ifdef NORMALLY_CONNECTABLE
    .normally_connectable = true,
#else
    .normally_connectable = false,
#endif

/**
 ****************************************************************************************
 * \brief Callback for storing CCC and attributes
 ****************************************************************************************
 */
#ifdef HAS_CONNECTION_FSM
    .store_attribute_callback = user_store_ccc,
#else
    .store_attribute_callback = NULL,
#endif
};

// Mouse Report Descriptor

// clang-format off

const uint8_t report_map[] = {

#if HID_WITH_KEYBOARD

    // Report: Standard keyboard

    0x05, 0x01,                     // Usage Page: Generic Desktop Controls
    0x09, 0x06,                     // Usage: Keyboard
    0xa1, 0x01,                     // Collection: Application
    0x85, HID_KEYBOARD_REPORT_ID,   // Report ID=N

    // modifier keys
    0x05, 0x07,                     // Usage Page: Keyboard
    0x19, 0xe0,                     // Usage Minimum: Keyboard LeftControl
    0x29, 0xe7,                     // Usage Maximum: Keyboard Right GUI
    0x15, 0x00,                     // Logical Minimum: 0
    0x25, 0x01,                     // Logical Maximum: 1
    0x75, 0x01,                     // Report Size: 1
    0x95, 0x08,                     // Report Count: 8
    0x81, 0x02,                     // Input: Data, Array, Absolute

    // reserved byte (required)
    0x95, 0x01,                     // Report Count: 1
    0x75, 0x08,                     // Report Size: 8
    0x81, 0x01,                     // Input: Constant, Array, Absolute

    // leds x 3
    0x95, 0x03,                     // Report Count: 3
    0x75, 0x01,                     // Report Size: 1
    0x05, 0x08,                     // Usage Page: LEDs
    0x19, 0x01,                     // Usage Minimum: Num Lock
    0x29, 0x03,                     // Usage Maximum: Scroll Lock
    0x91, 0x02,                     // Output: Data

    // pad bits x 5
    0x95, 0x05,                     // Report Count: 5
    0x75, 0x01,                     // Report Size: 1
    0x91, 0x01,                     // Output: Constant, Array, Absolute

    // key states x 2
    0x95, 0x02,                     // Report Count: # of key rollover
    0x75, 0x08,                     // Report Size: 8
    0x15, 0x00,                     // Logical Minimum: 0
    0x25, 0x65,                     // Logical Maximum: 101
    0x05, 0x07,                     // Usage Page: Keyboard/Keypad
    0x19, 0x00,                     // Usage Minimum: 0
    0x29, 0x65,                     // Usage Maximum: 101
    0x81, 0x00,                     // Input: Data, Var, Absolute

    0xc0,                           // End collection

#endif

#if HID_WITH_MOUSE

    // Report: Mouse

    0x05, 0x01,                    // Usage Page: Generic Desktop Controls
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, HID_MOUSE_REPORT_ID,     // Report ID=N
    0x09, 0x01,                    //   USAGE (Pointer)

    // 3 buttons
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //   USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //   REPORT_COUNT (3)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    // 5 pad bits
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x05,                    //   REPORT_SIZE (5)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    
    // 2 x 8 bit x,y
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //   USAGE (X)
    0x09, 0x31,                    //   USAGE (Y)
    0x15, 0x81,                    //   LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x02,                    //   REPORT_COUNT (2)
    0x81, 0x06,                    //   INPUT (Data,Var,Rel)

    0xc0,                          // END_COLLECTION

#endif

#if HID_WITH_CONSUMER

    // Report: Consumer Control Device

    0x05, 0x0c,                     // Usage Page (Consumer Devices)
	0x09, 0x01,                     // Usage (Consumer Control)
	0xa1, 0x01,                     // Collection (Application)
	0x85, HID_CONSUMER_REPORT_ID,	// Report ID=N
    
    // 8 x key states
	0x95, 0x08,                     // Report Count (8)
	0x75, 0x01,                     // Report Size (1)
	0x15, 0x00,                     // Logical Minimum (0)
	0x25, 0x01,                     // Logical Maximum (1)
	0x09, 0xb5,                     // Usage8 (Scan Next Track)
	0x09, 0xb6,                     // Usage8 (Scan Previous Track)
	0x09, 0xb7,                     // Usage8 (Stop)
	0x09, 0xcd,                     // Usage8 (Play / Pause)
	0x09, 0xe2,                     // Usage8 (Mute)
	0x09, 0xe9,                     // Usage8 (Volume Up)
	0x09, 0xea,                     // Usage8 (Volume Down)
    0x0a, 0xae, 0x01,               // Usage16 AL Keyboard Layout
	0x81, 0x02,                     // Input (Data, Variable, Absolute)

    0xc0,                           // End collection

#endif
};

// clang-format on

const int report_map_len = sizeof(report_map);

uint16_t report_ntf;
uint8_t hogpd_conidx;

#define REPORT_TO_MASK(index) (HOGPD_CFG_REPORT_NTF_EN << index)

#if HID_NUM_OF_REPORTS > HOGPD_NB_REPORT_INST_MAX
#error "Maximum munber of HID reports exceeded. Please increase HOGPD_NB_REPORT_INST_MAX"
#endif

void app_hogpd_enable(uint8_t conidx)
{
    arch_printf("app_hogpd_enable\n");
    struct hogpd_enable_req *req =
        KE_MSG_ALLOC(HOGPD_ENABLE_REQ, prf_get_task_from_id(TASK_ID_HOGPD), TASK_APP, hogpd_enable_req);
    hogpd_conidx = conidx;
    req->conidx = hogpd_conidx;
    report_ntf = 0;

    int i;
    for(i = 0; i < HID_NUM_OF_REPORTS; i++) {
        if((hogpd_reports[i].cfg & HOGPD_CFG_REPORT_IN) == HOGPD_CFG_REPORT_IN) {
            report_ntf |= REPORT_TO_MASK(i);
        }
    }
    req->ntf_cfg[0] = report_ntf;
    ke_msg_send(req);
}

void app_hogpd_create_db(void)
{
    struct hogpd_db_cfg *db_cfg;

    arch_printf("app_hogpd_create_db\n");

    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD, TASK_GAPM, TASK_APP,
                                                             gapm_profile_task_add_cmd, sizeof(struct hogpd_db_cfg));
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->prf_task_id = TASK_ID_HOGPD;
    req->app_task = TASK_APP;
    req->sec_lvl = get_user_prf_srv_perm(TASK_ID_HOGPD);
    req->start_hdl = 0;

    db_cfg = (struct hogpd_db_cfg *)req->param;
    struct hogpd_hids_cfg *cfg = &db_cfg->cfg[0];

    db_cfg->hids_nb = 1;

    struct hids_hid_info *hid_info = &cfg->hid_info;

    cfg->svc_features |= HOGPD_CFG_BOOT_KB_WR;

    cfg->report_nb = HID_NUM_OF_REPORTS;

    uint8_t i;
    for(i = 0; i < HID_NUM_OF_REPORTS; i++) {
        cfg->report_id[i] = hogpd_reports[i].id;
        cfg->report_char_cfg[i] = hogpd_reports[i].cfg;
    }


    for(i = HID_NUM_OF_REPORTS; i < HOGPD_NB_REPORT_INST_MAX; i++) {
        cfg->report_id[i] = 0;
        cfg->report_char_cfg[i] = 0;
    }

    hid_info->bcdHID = 0x100;
    hid_info->bCountryCode = 0;
    hid_info->flags = HIDS_REMOTE_WAKE_CAPABLE;

    ke_msg_send(req);
}

bool app_hogpd_send_report(uint8_t report_idx, uint8_t *data, uint16_t length, enum hogpd_report_type type)
{
    struct hogpd_report_upd_req *req;

    req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ, prf_get_task_from_id(TASK_ID_HOGPD), TASK_APP, hogpd_report_upd_req,
                           length);


    if(!req) {
        return false;
    }

    req->conidx = hogpd_conidx;

    struct hogpd_report_info *report = &req->report;

    // Fill in the parameter structure
    // TODO: find the index from the report number
    report->hid_idx = 0;

    ASSERT_ERROR((type != HOGPD_BOOT_KEYBOARD_INPUT_REPORT && type != HOGPD_BOOT_MOUSE_INPUT_REPORT) ||
                 report_idx == 0);
    ASSERT_ERROR((type != HOGPD_BOOT_KEYBOARD_INPUT_REPORT && type != HOGPD_BOOT_MOUSE_INPUT_REPORT) ||
                 length <= HOGPD_BOOT_REPORT_MAX_LEN);
    ASSERT_ERROR((type == HOGPD_BOOT_KEYBOARD_INPUT_REPORT || type == HOGPD_BOOT_MOUSE_INPUT_REPORT) ||
                 length <= HOGPD_REPORT_MAX_LEN);

    report->type = type;
    report->idx = report_idx;

    report->length = length;
    memcpy(report->value, data, length);

    ke_msg_send(req);

    return true;
}

uint8_t app_hogpd_get_protocol_mode(void)
{
    struct hogpd_env_tag *hogpd_env = PRF_ENV_GET(HOGPD, hogpd);
    return hogpd_env->svcs[0].proto_mode;
}

uint16_t app_hogpd_report_handle(uint8_t report_nb)
{
    ASSERT_WARNING(report_nb < HOGPD_NB_REPORT_INST_MAX);
    struct hogpd_env_tag *hogpd_env = PRF_ENV_GET(HOGPD, hogpd);
    return hogpd_get_att_handle(hogpd_env, 0, HOGPD_IDX_REPORT_VAL, report_nb);
}

#endif
