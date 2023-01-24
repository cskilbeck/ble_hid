#pragma once

#include "app_hogpd_defs.h"
#include "hogpd.h"
#include "hogpd_task.h"
#include "user_gamepad.h"

#define HID_KEYBOARD_REPORT_ID 1
#define HID_KEYBOARD_REPORT_SIZE 8

#define HID_AL_KEYBOARD_REPORT_ID 2
#define HID_AL_KEYBOARD_REPORT_SIZE 1

enum hogpd_indexes
{
    HID_KEYBOARD_REPORT_IDX = 0,
    HID_AL_KEYBOARD_REPORT_IDX,
    HID_NUM_OF_REPORTS    // Don't remove this.
};

extern const hogpd_reports_t hogpd_reports[HID_NUM_OF_REPORTS];

extern const hogpd_params_t hogpd_params;

extern const uint8_t report_map[];

extern const int report_map_len;
