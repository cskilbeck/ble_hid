//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#include "app_hogpd_defs.h"
#include "hogpd.h"
#include "hogpd_task.h"

//////////////////////////////////////////////////////////////////////

void app_hogpd_enable(uint8_t conidx);
void app_hogpd_create_db(void);
bool app_hogpd_send_report(uint8_t report_idx, uint8_t *data, uint16_t length, enum hogpd_report_type type);
uint8_t app_hogpd_get_protocol_mode(void);
uint16_t app_hogpd_report_handle(uint8_t report_nb);

//////////////////////////////////////////////////////////////////////

// Usage bits in the consumer control message

#define HID_CC_NEXT_TRACK (1 << 0)
#define HID_CC_PREV_TRACK (1 << 1)
#define HID_CC_STOP (1 << 2)
#define HID_CC_PLAY_PAUSE (1 << 3)
#define HID_CC_MUTE (1 << 4)
#define HID_CC_VOLUME_UP (1 << 5)
#define HID_CC_VOLUME_DOWN (1 << 6)
#define HID_CC_AL_KEYBOARD (1 << 7)

//////////////////////////////////////////////////////////////////////

#define WITH_KEYBOARD 0

#if WITH_KEYBOARD

#define HID_KEYBOARD_REPORT_ID 1
#define HID_KEYBOARD_REPORT_SIZE 8

#define HID_CONSUMER_REPORT_ID 2
#define HID_CONSUMER_REPORT_SIZE 1

enum hogpd_indexes
{
    HID_KEYBOARD_REPORT_IDX = 0,
    HID_CONSUMER_REPORT_IDX,
    HID_NUM_OF_REPORTS    // Don't remove this.
};

#else

#define HID_CONSUMER_REPORT_ID 1
#define HID_CONSUMER_REPORT_SIZE 2

enum hogpd_indexes
{
    HID_CONSUMER_REPORT_IDX = 0,
    HID_NUM_OF_REPORTS    // Don't remove this.
};

#endif

//////////////////////////////////////////////////////////////////////

extern const hogpd_reports_t hogpd_reports[HID_NUM_OF_REPORTS];

extern const hogpd_params_t hogpd_params;

extern const uint8_t report_map[];

extern const int report_map_len;

