//////////////////////////////////////////////////////////////////////

#include "rwip_config.h"
#include "gap.h"
#include "app_entry_point.h"
#include "app_hogpd.h"
#include "app_hogpd_task.h"
#include "app_easy_timer.h"
#include "user_peripheral.h"
#include "user_custs1_def.h"
#include "custs1_task.h"
#include "co_bt.h"
#include "prf.h"
#include "arch_console.h"
#include "user_periph_setup.h"
#include "../../../common/uart_message.h"

//////////////////////////////////////////////////////////////////////

timer_hnd app_param_update_request_timer_used __SECTION_ZERO("retention_mem_area0");
uint8_t app_connection_idx __SECTION_ZERO("retention_mem_area0");

bool button_notifications_enabled = false;

uint8_t uart1_buffer;
uint32_t uart_rx_data;
uint8_t uart_tx_data[4];

byte consumer_key_state;

//////////////////////////////////////////////////////////////////////

void print_uint32(char const *msg, uint32_t x)
{
    static char txt[10];

    for(int i = 0; i < 8; ++i) {
        uint32_t b = (x >> 28) + '0';
        if(b > '9') {
            b += 'A' - 10 - '0';
        }
        txt[i] = b;
        x <<= 4;
    }
    txt[8] = '\n';
    txt[9] = 0;
    arch_printf(msg);
    arch_printf(txt);
}

//////////////////////////////////////////////////////////////////////

void uart1_rx_callback(uint16_t data_cnt)
{
    byte got_byte = uart1_buffer;
    uart_receive(UART1, &uart1_buffer, 1, UART_OP_INTR);

    // if top bit is set, it's a checksum byte, grab it and reset stuff
    if((got_byte & 0x80) != 0) {

        uart_rx_data = got_byte;
        print_uint32("Got checksum: ", got_byte & 0xff);

    } else {

        // top bit is clear, it's a data byte, add it to the data and if got 3 check it
        uart_rx_data = (uart_rx_data << 8) | got_byte;

        if((uart_rx_data & 0x80000000) != 0) {

            uint32_t payload = um_decode_message(uart_rx_data);

            if(payload != 0xffffffff) {

                um_encode_message(payload, uart_tx_data);
                uart_send(UART1, uart_tx_data, 4, UART_OP_INTR);

                GPIO_TOGGLE(GPIO_LED_PORT, GPIO_LED_PIN);

                print_uint32("Got message ", payload);

                if(button_notifications_enabled) {
                    ble_control_point_send_payload(payload);
                }

                bool hid_send = false;

                if(UM_EXTRACT(payload, BTN1_PRESSED) != 0) {
                    consumer_key_state |= HID_CC_AL_KEYBOARD;
                    hid_send = true;
                }

                if(UM_EXTRACT(payload, BTN1_RELEASED) != 0) {
                    consumer_key_state &= ~HID_CC_AL_KEYBOARD;
                    hid_send = true;
                }

                if(UM_EXTRACT(payload, BTN2_PRESSED) != 0) {
                    consumer_key_state |= HID_CC_MUTE;
                    hid_send = true;
                }

                if(UM_EXTRACT(payload, BTN2_RELEASED) != 0) {
                    consumer_key_state &= ~HID_CC_MUTE;
                    hid_send = true;
                }

                int rot = UM_EXTRACT(payload, ROT1_ROTATED);

                if(rot == ROT_DIR_CLOCKWISE) {
                    consumer_key_state |= HID_CC_VOLUME_UP;
                    hid_send = true;
                } else if(rot == ROT_DIR_ANTICLOCKWISE) {
                    consumer_key_state |= HID_CC_VOLUME_DOWN;
                    hid_send = true;
                }

                if(hid_send) {
                    app_hogpd_send_report(HID_CONSUMER_REPORT_IDX, &consumer_key_state, HID_CONSUMER_REPORT_SIZE,
                                          HOGPD_REPORT);

                    if((consumer_key_state & (HID_CC_VOLUME_UP | HID_CC_VOLUME_DOWN)) != 0) {

                        consumer_key_state &= ~(HID_CC_VOLUME_UP | HID_CC_VOLUME_DOWN);

                        app_hogpd_send_report(HID_CONSUMER_REPORT_IDX, &consumer_key_state, HID_CONSUMER_REPORT_SIZE,
                                              HOGPD_REPORT);
                    }
                }

#if WITH_KEYBOARD
//                byte data[8];
//                memset(data, 0, 8);
//                if(((payload >> UM_BTN2_PRESSED_POS) & UM_BTN2_PRESSED_MASK) != 0) {
//                    data[3] = 5;
//                }
//                if(((payload >> UM_BTN2_RELEASED_POS) & UM_BTN2_RELEASED_MASK) != 0) {
//                    data[3] = 0;
//                }
#endif
                print_uint32("Good payload: ", payload);
            } else {
                print_uint32("Err, got ", uart_rx_data >> 24);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

void uart1_tx_callback(uint16_t data_cnt)
{
}

//////////////////////////////////////////////////////////////////////

void uart1_err_callback(uart_t *uart, uint8_t uart_err_status)
{
}

//////////////////////////////////////////////////////////////////////

void ble_control_point_send_payload(int32_t payload)
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
    // if(suota_state.reboot_requested) {
    //   platform_reset(RESET_AFTER_SUOTA_UPDATE);
    // }

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

        case SVC1_IDX_CONTROL_POINT_NTF_CFG: {
            button_notifications_enabled = msg_param->value[0] != 0;
            print_uint32("Configure notification:", msg_param->value[0]);
            break;
        }

        default:
            print_uint32("Huh (handle):", msg_param->handle);
            break;
        }
    } break;

    case GAPM_CMP_EVT:
        arch_printf("command complete\n");
        break;

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
        app_hogpd_process_handler(msgid, param, dest_id, src_id);
        break;
    }
}
