#pragma once

#include "hogpd.h"
#include "hogpd_task.h"

void app_hogpd_enable(uint8_t conidx);
void app_hogpd_create_db(void);
bool app_hogpd_send_report(uint8_t report_idx, uint8_t *data, uint16_t length, enum hogpd_report_type type);
uint8_t app_hogpd_get_protocol_mode(void);
uint16_t app_hogpd_report_handle(uint8_t report_nb);
