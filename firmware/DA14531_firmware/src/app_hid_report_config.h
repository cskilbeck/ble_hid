#pragma once

// Number of HID key reports that can be stored in the report FIFO

#define HID_REPORT_FIFO_SIZE 8

// Define HID_REPORT_LIST_FULL_WARNING to have an assertion in DEVELOPMENT_DEBUG 
// mode when the report FIFO is full

#undef HID_REPORT_FULL_WARNING

// Maximum HID report size that can be stored in the report FIFO

#define HID_REPORT_MAX_REPORT_SIZE 8

// Set the size of the rollover buffer. It must be greater or equal to the number
// of keys that can be detected as pressed at the same time.

#define HID_REPORT_ROLL_OVER_BUF_SIZE 7

// Enable reporting of any events happened while disconnected                        

#define HID_REPORT_HISTORY

#define HID_REPORT_NORMAL_REPORT_IDX    0
#define HID_REPORT_NORMAL_REPORT_SIZE   8
#define HID_REPORT_EXTENDED_REPORT_IDX  2
#define HID_REPORT_EXTENDED_REPORT_SIZE 3

