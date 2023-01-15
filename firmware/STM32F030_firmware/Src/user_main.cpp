//////////////////////////////////////////////////////////////////////
// if no button has been pressed for a while, go to sleep
// when button pressed, send message to BLE MCU
// when valid message received from BLE MCU, toggle LED

#include "main.h"

#if defined(DEBUG)
#define DISABLE_STANDBY
#endif

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks;      // 10Khz tick
button btn1;                // button1 - action / wakeup

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
    held = state != 0;
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
// send a 21 bit message (sends 32 bits with checksum and id bits)

void send_message(uint32 message)
{
    if(uart_tx_busy) {
        return;
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
        if(btn1.pressed) {
            idle_ticks = ticks;
            btn1.pressed = false;
            send_message(ticks);
            sent_payload = ticks & ((1 << 21) - 1);
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
        byte b = LL_USART_ReceiveData8(USART1); // reading the char clears the irq I guess?
        
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
