#pragma once

#include "ke_msg.h"
#include "hogpd.h"
#include "gattc_task.h"
#include "app_hogpd_defs.h"

/*
 *****************************************************************************************
 * Called when the Service Changed indication has been successfully received by the Host
 *          
 * param[in] msgid
 * param[in] param
 * param[in] dest_id
 * param[in] src_id
 *
 * return enum process_event_response
 *****************************************************************************************
 */
enum process_event_response app_hogpd_process_handler(ke_msg_id_t const msgid, void const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
