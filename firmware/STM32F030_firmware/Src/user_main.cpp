//////////////////////////////////////////////////////////////////////
// if no button has been pressed for a while, go to sleep
// when button pressed, send message to BLE MCU
// when valid message received from BLE MCU, toggle LED

#include "main.h"

#if defined(DEBUG)
#define DISABLE_STANDBY
#endif

//////////////////////////////////////////////////////////////////////

uint8 const led_gamma[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
    2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  11,  12,  12,  13,
    13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,
    24,  25,  25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,
    40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  54,  55,  56,  57,  58,  59,  60,  61,
    62,  63,  64,  66,  67,  68,  69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
    90,  92,  93,  95,  96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119, 120, 122, 124,
    126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167,
    169, 171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213, 215, 218,
    220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks;    // 10Khz tick
button btn1;              // button1 - action / wakeup

int32 sent_payload;
uint32 uart_rx_word;

volatile bool uart_tx_busy = false;
byte uart_tx_buffer[4];

static constexpr uint32 idle_standby_ticks = 10 * 10000;

//////////////////////////////////////////////////////////////////////
// button debouncer

void button::update(int state)
{
    history = (history << 1) | state;
    pressed |= history == bit_on_mask;
    released |= history == bit_off_mask;
    held |= pressed;
    held &= ~released;
}

//////////////////////////////////////////////////////////////////////
// go into standby mode, doesn't return, system reset on wakeup

void enter_standby_mode()
{
    LL_LPM_EnableDeepSleep();
    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);
    LL_PWR_ClearFlag_WU();
    __WFI();
}

//////////////////////////////////////////////////////////////////////
// clear up standby mode leavings at startup

void clear_standby_mode()
{
    LL_PWR_ClearFlag_SB();
    LL_PWR_ClearFlag_WU();
    LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN1);
}

//////////////////////////////////////////////////////////////////////

void set_led_rgb24(byte r, byte g, byte b)
{
    LL_TIM_OC_SetCompareCH1(TIM3, led_gamma[b]);
    LL_TIM_OC_SetCompareCH2(TIM3, led_gamma[g]);
    LL_TIM_OC_SetCompareCH4(TIM3, led_gamma[r]);
}

//////////////////////////////////////////////////////////////////////
// send a 21 bit message (sends 32 bits with checksum and id bits)

int32 send_message(uint32 message)
{
    if(uart_tx_busy) {
        return -1;
    }
    uart_tx_busy = true;
    int d0 = message & 0x7f;
    int d1 = (message >> 7) & 0x7f;
    int d2 = (message >> 14) & 0x7f;
    uart_tx_buffer[0] = (d0 ^ 0x7f ^ d1 ^ d2 ^ 0x55) | 0x80;
    uart_tx_buffer[1] = d2;
    uart_tx_buffer[2] = d1;
    uart_tx_buffer[3] = d0;
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 4);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32)uart_tx_buffer);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    return message & ((1 << 21) - 1);
}

//////////////////////////////////////////////////////////////////////
// c = Checksum
// p = Payload
// Input
// 31.............................0
// 1ccccccc0ppppppp0ppppppp0ppppppp
// Output Success
// 00000000000ppppppppppppppppppppp
// Output Fail
// 11111111111111111111111111111111

int32 decode_message(uint32 data)
{
    byte d0 = data;
    byte d1 = data >> 8;
    byte d2 = data >> 16;
    byte checksum = (data >> 24) & 0x7f;
    if(checksum == (d0 ^ 0x7f ^ d1 ^ d2 ^ 0x55)) {
        return (d2 << 14) | (d1 << 7) | d0;
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////

extern "C" void user_main(void)
{
    SysTick->CTRL = 0;
    DEBUG_LED_GPIO_Port->BSRR = DEBUG_LED_Pin << 16;

    clear_standby_mode();

    // start timer14 (ticks + buttons)
    LL_TIM_EnableIT_UPDATE(TIM14);
    LL_TIM_EnableCounter(TIM14);

    // start timer3 (rgb led)
    set_led_rgb24(0, 0, 0);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH4);
    LL_TIM_EnableCounter(TIM3);
    LL_TIM_EnableAllOutputs(TIM3);

    // start uart (send/recv to/from BLE MCU)
    LL_USART_EnableIT_TC(USART1);
    LL_USART_EnableIT_RXNE(USART1);
    LL_USART_EnableDirectionTx(USART1);
    LL_USART_EnableDirectionRx(USART1);

    // setup uart DMA
    LL_USART_EnableDMAReq_TX(USART1);
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_2, LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_TRANSMIT));
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);

    ticks = 0;

    // rapid led flash until button is released
    while((BTN1_WAKE_GPIO_Port->IDR & BTN1_WAKE_Pin) != 0) {
        DEBUG_LED_GPIO_Port->BSRR = DEBUG_LED_Pin << (((ticks >> 9) & 1) << 4);
        __WFI();
    }

    // main loop
    uint32 idle_ticks = ticks;
    uint32 i = 0;
    while(1) {
        set_led_rgb24(ticks >> 6, ticks >> 7, ticks >> 8);
        bool send_it = false;
        if(btn1.pressed) {
            btn1.pressed = 0;
            send_it = true;
        }
        if(btn1.released) {
            btn1.released = 0;
            send_it = true;
        }
        if(send_it) {
            idle_ticks = ticks;
            sent_payload = send_message(((ticks & 0xff) << 8) | btn1.held);
        }
#if !defined(DISABLE_STANDBY)
        if((ticks - idle_ticks) > idle_standby_ticks) {
            enter_standby_mode();
        }
#endif
        __WFI();
    }
}

//////////////////////////////////////////////////////////////////////

extern "C" void uart_irq_handler(void)
{
    // done transmitting via DMA?
    if(LL_USART_IsActiveFlag_TC(USART1)) {
        LL_USART_ClearFlag_TC(USART1);
        uart_tx_busy = false;
    }

    // received a byte?
    if(LL_USART_IsActiveFlag_RXNE(USART1)) {
        byte b = LL_USART_ReceiveData8(USART1);    // reading the char clears the irq I guess?

        // top bit set means it's a checksum, which comes first
        if((b & 0x80) != 0) {
            uart_rx_word = b;
        } else {
            // else copy the byte into the buffer
            uart_rx_word = (uart_rx_word << 8) | b;

            // checksum made it to the top?
            if((uart_rx_word & 0x80000000) != 0) {

                // check it
                int32 payload = decode_message(uart_rx_word);

                // checksum checked out?
                if(payload == sent_payload) {
                    DEBUG_LED_GPIO_Port->ODR ^= DEBUG_LED_Pin;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////

extern "C" void timer14_irq_handler(void)
{
    LL_TIM_ClearFlag_UPDATE(TIM14);
    btn1.update((BTN1_WAKE_GPIO_Port->IDR & BTN1_WAKE_Pin) != 0);
    ticks += 1;
}

//////////////////////////////////////////////////////////////////////

extern "C" void dma_channels_2_3_irq_handler(void)
{
    if(LL_DMA_IsActiveFlag_TC2(DMA1)) {
        LL_DMA_ClearFlag_TC2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    }
    if(LL_DMA_IsActiveFlag_TC3(DMA1)) {
        LL_DMA_ClearFlag_TC3(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    }
}
