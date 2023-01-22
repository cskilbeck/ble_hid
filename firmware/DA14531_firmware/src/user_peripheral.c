//////////////////////////////////////////////////////////////////////

#include "rwip_config.h"    // SW configuration
#include "gap.h"
#include "app_easy_timer.h"
#include "user_peripheral.h"
#include "user_custs1_def.h"
#include "custs1_task.h"
#include "co_bt.h"
#include "prf.h"
#include "arch_console.h"

//////////////////////////////////////////////////////////////////////

timer_hnd app_param_update_request_timer_used __SECTION_ZERO("retention_mem_area0");
uint8_t app_connection_idx __SECTION_ZERO("retention_mem_area0");

bool button_notifications_enabled = false;

void print_uint32(uint32_t x);

//////////////////////////////////////////////////////////////////////

void send_payload(int32_t payload)
{
    struct custs1_val_ntf_ind_req *req =
        KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ, prf_get_task_from_id(TASK_ID_CUSTS1), TASK_APP, custs1_val_ntf_ind_req,
                         DEF_SVC1_CTRL_POINT_CHAR_LEN);

    req->handle = SVC1_IDX_CONTROL_POINT_VAL;
    req->length = 4;
    req->notification = true;
    memcpy(req->value, &payload, 4);
    ke_msg_send(req);
}

//////////////////////////////////////////////////////////////////////

static void param_update_request_timer_cb()
{
    arch_printf("param_update_request\n");
    app_easy_gap_param_update_start(app_connection_idx);
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
}

//////////////////////////////////////////////////////////////////////

void user_app_init(void)
{
    arch_printf("user_app_init\n");
    default_app_on_init();
}

//////////////////////////////////////////////////////////////////////

void user_app_on_db_init_complete(void)
{
    arch_printf("on_db_init_complete\n");
    default_app_on_db_init_complete();
}

//////////////////////////////////////////////////////////////////////

void user_app_adv_start(void)
{
    arch_printf("user_app_adv_start\n");
    app_easy_gap_undirected_advertise_start();
}

//////////////////////////////////////////////////////////////////////

void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if(app_env[connection_idx].conidx != GAP_INVALID_CONIDX) {

        arch_printf("user_app_connection\n");

        app_connection_idx = connection_idx;

        // Check if the parameters of the established connection are the preferred ones.
        // If not then schedule a connection parameter update request.
        if((param->con_interval < user_connection_param_conf.intv_min) ||
           (param->con_interval > user_connection_param_conf.intv_max) ||
           (param->con_latency != user_connection_param_conf.latency) ||
           (param->sup_to != user_connection_param_conf.time_out)) {
            // Connection params are not these that we expect
            app_param_update_request_timer_used =
                app_easy_timer(APP_PARAM_UPDATE_REQUEST_TO, param_update_request_timer_cb);
        }
    } else {
        arch_printf("invalid connection!?\n");
        // No connection has been established, restart advertising
        user_app_adv_start();
    }

    default_app_on_connection(connection_idx, param);
}

//////////////////////////////////////////////////////////////////////

void user_app_adv_undirect_complete(uint8_t status)
{
    arch_printf("undirect_complete\n");

    // If advertising was canceled then update advertising data and start advertising again
    if(status == GAP_ERR_CANCELED) {
        user_app_adv_start();
    }
}

//////////////////////////////////////////////////////////////////////

void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
    //if(suota_state.reboot_requested) {
    //  platform_reset(RESET_AFTER_SUOTA_UPDATE);
    //}

    arch_printf("app_disconnect\n");
    // Cancel the parameter update request timer
    if(app_param_update_request_timer_used != EASY_TIMER_INVALID_TIMER) {
        app_easy_timer_cancel(app_param_update_request_timer_used);
        app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
    }
    // Restart Advertising
    user_app_adv_start();
}

//////////////////////////////////////////////////////////////////////

void user_custs1_server_rx_ind_handler(ke_msg_id_t const msgid, struct custs1_val_write_ind const *param,
                                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    arch_printf("rx_ind\n");
}

//////////////////////////////////////////////////////////////////////

void user_custs1_server_tx_cfg_ind_handler(ke_msg_id_t const msgid, struct custs1_val_write_ind const *param,
                                           ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    arch_printf("tx_cfg_ind\n");
    // CCC value already handled in cust1 task
}

//////////////////////////////////////////////////////////////////////

void user_custs1_server_tx_ntf_cfm_handler(ke_msg_id_t const msgid, struct custs1_val_ntf_cfm const *param,
                                           ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    arch_printf("tx_ntf_cf\n");
}



//////////////////////////////////////////////////////////////////////

void user_catch_rest_hndl(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    switch(msgid) {
    // Check if the request is a Write Indication
    case CUSTS1_VAL_WRITE_IND: {

        struct custs1_val_write_ind const *msg_param = (struct custs1_val_write_ind const *)(param);

        switch(msg_param->handle) {

        // Check if the message was destined to our custom service and if it matches the expected request
        case SVC1_IDX_CONTROL_POINT_VAL:
            if(msg_param->length == 1 && msg_param->value[0] == 'A') {

                struct custs1_val_ntf_ind_req *req =
                    KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ, prf_get_task_from_id(TASK_ID_CUSTS1), TASK_APP,
                                     custs1_val_ntf_ind_req, DEF_SVC1_CTRL_POINT_CHAR_LEN);
                req->handle = SVC1_IDX_CONTROL_POINT_VAL;
                req->length = 1;
                req->notification = true;
                req->value[0] = 'B';
                ke_msg_send(req);
                arch_printf("Got A, sent B!\n");
            }
            break;

        case SVC1_IDX_CONTROL_POINT_NTF_CFG:
        {
            arch_printf("Configure notification:");
            button_notifications_enabled = msg_param->value[0] != 0;
            print_uint32(msg_param->value[0]);
            break;
        }

        default:
            arch_printf("Huh:");
            print_uint32(msg_param->handle);
            break;
        }
    } break;


    case GAPC_PARAM_UPDATED_IND: {
        // Cast the "param" pointer to the appropriate message structure
        struct gapc_param_updated_ind const *msg_param = (struct gapc_param_updated_ind const *)(param);

        // Check if updated Conn Params filled to preferred ones
        if((msg_param->con_interval >= user_connection_param_conf.intv_min) &&
           (msg_param->con_interval <= user_connection_param_conf.intv_max) &&
           (msg_param->con_latency == user_connection_param_conf.latency) &&
           (msg_param->sup_to == user_connection_param_conf.time_out)) {
            arch_printf("params updated ok\n");
        }
    } break;

    default:
        arch_printf("Huh:");
        print_uint32(msgid);
        break;
    }
}
