//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////

#define READ_GPIO(port, mask) (((port)->IDR & mask) != 0)

//////////////////////////////////////////////////////////////////////

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed int int32;
typedef signed short int16;
typedef signed char int8;

typedef uint8 byte;

//////////////////////////////////////////////////////////////////////

void user_main(void);
void uart_irq_handler(void);
void timer14_irq_handler(void);
void dma_channels_2_3_irq_handler(void);

//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

//////////////////////////////////////////////////////////////////////
// button, call update at ~5..10KHz

struct button
{
    static constexpr int bit_history_len = 7;
    static constexpr uint32 bit_off_mask = (1 << bit_history_len) - 2;
    static constexpr uint32 bit_on_mask = 1;

    uint32 history : bit_history_len;
    uint32 held : 1;
    uint32 pressed : 1;
    uint32 released : 1;

    // returns true if state changed (pressed or released)
    // param: state is button state 0 or 1 (1 = button down)
    bool update(int state);
};

//////////////////////////////////////////////////////////////////////
// rotary encoder, call update at ~5..10KHz

struct rotary_encoder
{
    int state = 0;
    uint8 store = 0;

    // returns direction (-1, 0, 1)
    // param: inputs is 2 bits of encoder A/B
    int update(int inputs);
};

#endif

//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
