//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#include "app_hogpd_defs.h"
#include "hogpd.h"
#include "hogpd_task.h"

#define STATIC_ASSERT(x) extern char static_assert_foo[x];

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

#define HID_WITH_KEYBOARD 1
#define HID_WITH_MOUSE 1
#define HID_WITH_CONSUMER 1

STATIC_ASSERT(HID_WITH_CONSUMER || HID_WITH_KEYBOARD || HID_WITH_MOUSE);

//////////////////////////////////////////////////////////////////////

enum HID_REPORT_IDs
{
    HID_NULL_REPORT_ID = 0,

#if HID_WITH_KEYBOARD
    HID_KEYBOARD_REPORT_ID,
#endif

#if HID_WITH_MOUSE
    HID_MOUSE_REPORT_ID,
#endif

#if HID_WITH_CONSUMER
    HID_CONSUMER_REPORT_ID
#endif
};

//////////////////////////////////////////////////////////////////////

enum hogpd_indexes
{
#if HID_WITH_KEYBOARD
    HID_KEYBOARD_REPORT_IDX,
#endif

#if HID_WITH_MOUSE
    HID_MOUSE_REPORT_IDX,
#endif

#if HID_WITH_CONSUMER
    HID_CONSUMER_REPORT_IDX,
#endif

    HID_NUM_OF_REPORTS    // Don't remove this.
};

//////////////////////////////////////////////////////////////////////

// Format and size of hid reports is determined by the descriptors in app_hogpd.c / report_map[]

#pragma pack(push, 1)

struct hid_keyboard_report_t
{
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t key_state[2];
};

struct hid_mouse_report_t
{
    uint8_t buttons;
    int8_t x;
    int8_t y;
};

struct hid_consumer_report_t
{
    uint8_t key_states;
};

#pragma pack(pop)

#define HID_KEYBOARD_REPORT_SIZE 4
#define HID_MOUSE_REPORT_SIZE 3
#define HID_CONSUMER_REPORT_SIZE 1

STATIC_ASSERT(sizeof(struct hid_keyboard_report_t) == HID_KEYBOARD_REPORT_SIZE);
STATIC_ASSERT(sizeof(struct hid_mouse_report_t) == HID_MOUSE_REPORT_SIZE);
STATIC_ASSERT(sizeof(struct hid_consumer_report_t) == HID_CONSUMER_REPORT_SIZE);

//////////////////////////////////////////////////////////////////////

extern const hogpd_reports_t hogpd_reports[HID_NUM_OF_REPORTS];

extern const hogpd_params_t hogpd_params;

extern const uint8_t report_map[];
extern const int report_map_len;
