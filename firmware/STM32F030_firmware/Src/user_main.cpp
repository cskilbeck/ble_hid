//////////////////////////////////////////////////////////////////////
//
// add HOG profile and make it work (BLE volume knob)
// BLE reconnect
// sleep/standby/wakeup for both
// get the ADC working
// add BAT service and make it work (can see it in the BLE Mobile app)
//
//////////////////////////////////////////////////////////////////////

// if no button has been pressed for a while, go to sleep
// when button pressed, send message to BLE MCU
// when valid message received from BLE MCU, toggle LED

#include "main.h"
#include "../../common/uart_message.h"

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

volatile uint32 ticks;       // 10Khz tick
uint32 idle_ticks;           // last activity timestamp
button btn1;                 // button1 - action / wakeup
button btn2;                 // button2 - encoder 0 click
rotary_encoder encoder_0;    // 1st rotary encoder

int32 sent_payload;
uint32 uart_rx_word;

volatile bool uart_tx_busy = false;
byte uart_tx_buffer[4];

static constexpr uint32 idle_standby_ticks = 10 * 10000;

//////////////////////////////////////////////////////////////////////
// rotary encoder reader

int rotary_encoder::update(int inputs)
{
    //////////////////////////////////////////////////////////////////////
    // Valid transitions are:
    // 1    00 .. 01
    // 2    00 .. 10
    // 4    01 .. 00
    // 7    01 .. 11
    // 8    10 .. 00
    // 11   10 .. 11
    // 13   11 .. 01
    // 14   11 .. 10

    // bitmask of which 2-state histories are valid (see table above)
    static constexpr uint16 valid_rotary_state_mask = 0x6996;

    // then, to just get one increment per cycle:

    // 11 .. 10 .. 00 is one way
    // 00 .. 10 .. 11 is the other way

    // So:
    // E8 = 11,10 .. 10,00  --> one way
    // 2B = 00,10 .. 10,11  <-- other way

    static constexpr int ROTARY_CLOCKWISE = 0xE8;
    static constexpr int ROTARY_ANTICLOCKWISE = 0x2B;

    state = ((state << 2) | inputs) & 0xf;

    // many states are invalid (noisy switches) so ignore them
    if((valid_rotary_state_mask & (1 << state)) != 0) {
        // certain state patterns mean rotation happened
        store = (store << 4) | state;
        switch(store) {
        case ROTARY_CLOCKWISE:
            return 1;
        case ROTARY_ANTICLOCKWISE:
            return -1;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////
// button debouncer

bool button::update(int state)
{
    history = (history << 1) | state;
    pressed = history == bit_on_mask;
    released = history == bit_off_mask;
    return pressed || released;
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
    int32 sent = um_encode_message(message, uart_tx_buffer);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 4);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32)uart_tx_buffer);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    return sent;
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
    
    // at this point, we could set the SWD pins as GPIOs...

    // main loop
    while(1) {
        __WFI();
    }
}

//////////////////////////////////////////////////////////////////////

extern "C" void uart_irq_handler(void)
{
    // done transmitting last byte?
    if(LL_USART_IsActiveFlag_TC(USART1)) {
        LL_USART_ClearFlag_TC(USART1);
        uart_tx_busy = false;
    }

    // received a byte?
    if(LL_USART_IsActiveFlag_RXNE(USART1)) {

        // reading the char clears the irq I guess?
        byte b = LL_USART_ReceiveData8(USART1);

        // top bit set means it's a checksum, which comes first
        if((b & 0x80) != 0) {

            uart_rx_word = b;

        } else {

            // else copy the byte into the buffer
            uart_rx_word = (uart_rx_word << 8) | b;

            // checksum made it to the top?
            if((uart_rx_word & 0x80000000) != 0) {

                // check it
                int32 payload = um_decode_message(uart_rx_word);

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

    bool send = false;

    int btn1_state = READ_GPIO(BTN1_WAKE_GPIO_Port, BTN1_WAKE_Pin);
    
    // invert because it's a pull-up
    int btn2_state = 1 - READ_GPIO(BTN2_GPIO_Port, BTN2_Pin);

    send |= btn1.update(btn1_state);
    send |= btn2.update(btn2_state);

    int ch_a = READ_GPIO(ENC0_A_GPIO_Port, ENC0_A_Pin);
    int ch_b = READ_GPIO(ENC0_B_GPIO_Port, ENC0_B_Pin);

    int encoder_0_direction = encoder_0.update(ch_b | (ch_a << 1));

    send |= encoder_0_direction != 0;

    if(send) {

        uint32 data = 0;
        
        data |= (btn1.was_pressed() & UM_BTN1_PRESSED_MASK) << UM_BTN1_PRESSED_POS;
        data |= (btn1.was_released() & UM_BTN1_RELEASED_MASK) << UM_BTN1_RELEASED_POS;

        data |= (btn2.was_pressed() & UM_BTN2_PRESSED_MASK) << UM_BTN2_PRESSED_POS;
        data |= (btn2.was_released() & UM_BTN2_RELEASED_MASK) << UM_BTN2_RELEASED_POS;

        //data |= (btn3.pressed & UM_BTN3_PRESSED_MASK) << UM_BTN3_PRESSED_POS;
        //data |= (btn3.released & UM_BTN3_PRESSED_MASK) << UM_BTN3_PRESSED_POS;

        data |= (encoder_0_direction & UM_ROT1_ROTATED_MASK) << UM_ROT1_ROTATED_POS;

        //data |= (encoder_1_direction & UM_ROT2_ROTATED_MASK) << UM_ROT2_ROTATED_POS;

        sent_payload = send_message(data);
    }
    ticks += 1;

#if !defined(DISABLE_STANDBY)
    if((ticks - idle_ticks) > idle_standby_ticks) {
        enter_standby_mode();
    }
#endif
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
