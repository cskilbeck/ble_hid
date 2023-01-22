//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#include "rwble_config.h"
#include "app_task.h"     // application task
#include "gapc_task.h"    // gap functions and messages
#include "gapm_task.h"    // gap functions and messages
#include "app.h"          // application definitions
#include "co_error.h"     // error code definitions
#include "arch_wdg.h"

#include "app_callback.h"
#include "app_default_handlers.h"

//////////////////////////////////////////////////////////////////////

/* Duration of timer for connection parameter update request */
#define APP_PARAM_UPDATE_REQUEST_TO \
    (1000)    // 1000*10ms = 10sec, The maximum allowed value is 41943sec (4194300 * 10ms)

/* Advertising data update timer */
#define APP_ADV_DATA_UPDATE_TO (3000)    // 3000*10ms = 30sec, The maximum allowed value is 41943sec (4194300 * 10ms)

/* Manufacturer specific data constants */
#define APP_AD_MSD_COMPANY_ID (0xABCD)
#define APP_AD_MSD_COMPANY_ID_LEN (2)
#define APP_AD_MSD_DATA_LEN (sizeof(uint16_t))

#define APP_PERIPHERAL_CTRL_TIMER_DELAY 100

//////////////////////////////////////////////////////////////////////

extern bool button_notifications_enabled;

//////////////////////////////////////////////////////////////////////

void user_app_init(void);

void user_app_adv_start(void);

//////////////////////////////////////////////////////////////////////
// BLE connected
// connection_idx   : Connection Id index
// param            : Pointer to GAPC_CONNECTION_REQ_IND message

void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param);

//////////////////////////////////////////////////////////////////////
// Undirect advertising completion function.
// status           : Command complete event message status

void user_app_adv_undirect_complete(uint8_t status);

//////////////////////////////////////////////////////////////////////
// BLE disconnected
// param            : Pointer to GAPC_DISCONNECT_IND message

void user_app_disconnect(struct gapc_disconnect_ind const *param);

//////////////////////////////////////////////////////////////////////
// Handles the messages that are not handled by the SDK internal mechanisms.
// msgid            : Id of the message received.
// param            : Pointer to the parameters of the message.
// dest_id          : ID of the receiving task instance.
// src_id           : ID of the sending task instance.

void user_catch_rest_hndl(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);

//////////////////////////////////////////////////////////////////////
// Service database initialization was completed

void user_app_on_db_init_complete(void);
