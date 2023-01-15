//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

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

// PULLUP button, call update at ~1..10KHz
// clear pressed/released flags after reading them

struct button
{
    static constexpr int bit_history_len = 4;
    static constexpr uint32 bit_off_mask = (1 << bit_history_len) - 2;
    static constexpr uint32 bit_on_mask = 1;

    uint32 history : bit_history_len;
    uint32 held : 1;
    uint32 pressed : 1;
    uint32 released : 1;

    void update(int state);    // state is GPIO input shifted into bit 0 position
};

#endif

//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
