//////////////////////////////////////////////////////////////////////

#pragma once

#include "int_types.h"

//////////////////////////////////////////////////////////////////////
// Format of a uart message from STM32 -> DA14531 is:
// BITS
//  0       BTN1 Pressed
//  1       BTN1 Released
//  2       BTN2 Pressed
//  3       BTN2 Released
//  4       BTN3 Pressed
//  5       BTN3 Released
//  6..7    ROT1 Rotation   (01 = clockwise, 11 = anti-clockwise)
//  8..9    ROT2 Rotation   (01 = clockwise, 11 = anti-clockwise)

#define UM_BTN1_PRESSED_POS 0
#define UM_BTN1_RELEASED_POS 1
#define UM_BTN2_PRESSED_POS 2
#define UM_BTN2_RELEASED_POS 3
#define UM_BTN3_PRESSED_POS 4
#define UM_BTN3_RELEASED_POS 5
#define UM_ROT1_ROTATED_POS 6
#define UM_ROT2_ROTATED_POS 8

#define UM_BTN1_PRESSED_MASK 1
#define UM_BTN1_RELEASED_MASK 1
#define UM_BTN2_PRESSED_MASK 1
#define UM_BTN2_RELEASED_MASK 1
#define UM_BTN3_PRESSED_MASK 1
#define UM_BTN3_RELEASED_MASK 1
#define UM_ROT1_ROTATED_MASK 3
#define UM_ROT2_ROTATED_MASK 3

#define UM_EXTRACT(data, name) ((data >> UM_ ## name ## _POS) & UM_ ## name ## _MASK)

#define ROT_DIR_CLOCKWISE 1
#define ROT_DIR_ANTICLOCKWISE 3

//////////////////////////////////////////////////////////////////////
// c = Checksum
// p = Payload
//
// Transmission format, 4 bytes
// [0] 1ccccccc
// [1] 0ppppppp
// [2] 0ppppppp
// [3] 0ppppppp
//
// Payload, 21 bits
// 00000000000ppppppppppppppppppppp

//////////////////////////////////////////////////////////////////////

static inline byte um_checksum(byte d0, byte d1, byte d2)
{
    return d0 ^ 0x7f ^ d1 ^ d2 ^ 0x55;
}

//////////////////////////////////////////////////////////////////////

static inline int32 um_decode_message(uint32 data)
{
    byte d0 = data;
    byte d1 = data >> 8;
    byte d2 = data >> 16;
    byte checksum_received = (data >> 24) & 0x7f;
    if(um_checksum(d0, d1, d2) == checksum_received) {
        return (d2 << 14) | (d1 << 7) | d0;
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////

static inline int32 um_encode_message(uint32 message, uint8 buffer[4])
{
    int d0 = message & 0x7f;
    int d1 = (message >> 7) & 0x7f;
    int d2 = (message >> 14) & 0x7f;
    buffer[0] = um_checksum(d0, d1, d2) | 0x80;
    buffer[1] = d2;
    buffer[2] = d1;
    buffer[3] = d0;
    return message & ((1 << 21) - 1);
}
