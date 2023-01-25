//////////////////////////////////////////////////////////////////////

#include "app_hogpd.h"
#include "user_periph_setup.h"
#include "user_peripheral.h"
#include "user_hogpd_config.h"
#include "datasheet.h"
#include "system_library.h"
#include "rwip_config.h"
#include "gpio.h"
#include "uart.h"
#include "syscntl.h"
#include "user_custs1_def.h"
#include "custs1_task.h"
#include "arch_console.h"
#include "../../../common/uart_message.h"

//////////////////////////////////////////////////////////////////////

static void uart1_err_callback(uart_t *uart, uint8_t uart_err_status);
static void uart1_tx_callback(uint16_t data_cnt);
static void uart1_rx_callback(uint16_t data_cnt);

//////////////////////////////////////////////////////////////////////

uint8_t uart1_buffer;
uint32_t uart_rx_data;
uint8_t uart_tx_data[4];

// Configuration struct for UART1 (used to talk to the STM32 Button handler MCU)
static const uart_cfg_t uart1_cfg = { .baud_rate = UART1_BAUDRATE,
                                      .data_bits = UART1_DATABITS,
                                      .parity = UART1_PARITY,
                                      .stop_bits = UART1_STOPBITS,
                                      .auto_flow_control = UART1_AFCE,
                                      .use_fifo = UART1_FIFO,
                                      .tx_fifo_tr_lvl = UART1_TX_FIFO_LEVEL,
                                      .rx_fifo_tr_lvl = UART1_RX_FIFO_LEVEL,
                                      .intr_priority = 2,
                                      .uart_rx_cb = uart1_rx_callback,
                                      .uart_err_cb = uart1_err_callback,
                                      .uart_tx_cb = uart1_tx_callback };

//////////////////////////////////////////////////////////////////////

#if defined(CFG_PRINTF_UART2)

// Configuration struct for UART2 (used to send debug messages to Putty)
static const uart_cfg_t uart_cfg = { .baud_rate = UART2_BAUDRATE,
                                     .data_bits = UART2_DATABITS,
                                     .parity = UART2_PARITY,
                                     .stop_bits = UART2_STOPBITS,
                                     .auto_flow_control = UART2_AFCE,
                                     .use_fifo = UART2_FIFO,
                                     .tx_fifo_tr_lvl = UART2_TX_FIFO_LEVEL,
                                     .rx_fifo_tr_lvl = UART2_RX_FIFO_LEVEL,
                                     .intr_priority = 2,
                                     .uart_rx_cb = NULL,
                                     .uart_err_cb = NULL,
                                     .uart_tx_cb = NULL };
#endif

//////////////////////////////////////////////////////////////////////

#if DEVELOPMENT_DEBUG

void GPIO_reservations(void)
{
#if defined(CFG_PRINTF_UART2)
    RESERVE_GPIO(UART2_TX, UART2_TX_PORT, UART2_TX_PIN, PID_UART2_TX);
#endif

#if !defined(__DA14586__) && !defined(__DA14531__)
    RESERVE_GPIO(SPI_EN, SPI_EN_PORT, SPI_EN_PIN, PID_SPI_EN);
#endif

    RESERVE_GPIO(UART1_RX, UART1_RX_PORT, UART1_RX_PIN, PID_UART1_RX);
    RESERVE_GPIO(UART1_TX, UART1_TX_PORT, UART1_TX_PIN, PID_UART1_TX);
    RESERVE_GPIO(LED, GPIO_LED_PORT, GPIO_LED_PIN, PID_GPIO);
}

#endif

//////////////////////////////////////////////////////////////////////

void set_pad_functions(void)
{
#if defined(__DA14586__)
    // Disallow spontaneous DA14586 SPI Flash wake-up
    GPIO_ConfigurePin(GPIO_PORT_2, GPIO_PIN_3, OUTPUT, PID_GPIO, true);
#elseif !defined(__DA14531__)
    GPIO_ConfigurePin(SPI_EN_PORT, SPI_EN_PIN, OUTPUT, PID_SPI_EN, true);
#endif

#if defined(CFG_PRINTF_UART2)
    GPIO_ConfigurePin(UART2_TX_PORT, UART2_TX_PIN, OUTPUT, PID_UART2_TX, false);
#endif

    GPIO_ConfigurePin(GPIO_LED_PORT, GPIO_LED_PIN, OUTPUT, PID_GPIO, false);
    GPIO_ConfigurePin(UART1_RX_PORT, UART1_RX_PIN, INPUT, PID_UART1_RX, false);
    GPIO_ConfigurePin(UART1_TX_PORT, UART1_TX_PIN, OUTPUT, PID_UART1_TX, false);
}

//////////////////////////////////////////////////////////////////////

void print_uint32(char const *msg, uint32_t x)
{
    static char txt[10];

    for(int i = 0; i < 8; ++i) {
        uint32_t b = (x >> 28) + '0';
        if(b > '9') {
            b += 'A' - 10 - '0';
        }
        txt[i] = b;
        x <<= 4;
    }
    txt[8] = '\n';
    txt[9] = 0;
    arch_printf(msg);
    arch_printf(txt);
}

//////////////////////////////////////////////////////////////////////

static void uart1_rx_callback(uint16_t data_cnt)
{
    byte got_byte = uart1_buffer;
    uart_receive(UART1, &uart1_buffer, 1, UART_OP_INTR);

    // if top bit is set, it's a checksum byte, grab it and reset stuff
    if((got_byte & 0x80) != 0) {

        uart_rx_data = got_byte;
        print_uint32("Got checksum: ", got_byte & 0xff);

    } else {

        // top bit is clear, it's a data byte, add it to the data and if got 3 check it
        uart_rx_data = (uart_rx_data << 8) | got_byte;

        if((uart_rx_data & 0x80000000) != 0) {

            uint32_t payload = um_decode_message(uart_rx_data);

            if(payload != 0xffffffff) {

                um_encode_message(payload, uart_tx_data);
                uart_send(UART1, uart_tx_data, 4, UART_OP_INTR);
                GPIO_TOGGLE(GPIO_LED_PORT, GPIO_LED_PIN);
                print_uint32("Got message ", payload);
                if(button_notifications_enabled) {
                    ble_control_point_send_payload(payload);
                }
                byte data[8];
                memset(data, 0, 8);
                if(((payload >> UM_BTN1_PRESSED_POS) & UM_BTN1_PRESSED_MASK) != 0) {
                    data[2] = 4;
                }
                if(((payload >> UM_BTN1_RELEASED_POS) & UM_BTN1_RELEASED_MASK) != 0) {
                    data[2] = 0;
                }
                if(((payload >> UM_BTN2_PRESSED_POS) & UM_BTN2_PRESSED_MASK) != 0) {
                    data[3] = 5;
                }
                if(((payload >> UM_BTN2_RELEASED_POS) & UM_BTN2_RELEASED_MASK) != 0) {
                    data[3] = 0;
                }
                int rot1 = (payload >> UM_ROT1_ROTATED_POS) & UM_ROT1_ROTATED_MASK;
                switch(rot1) {
                    case 1:
                        data[4] = 6;
                        break;
                    case 3:
                        data[4] = 7;
                        break;
                }
                app_hogpd_send_report(HID_KEYBOARD_REPORT_IDX, data, 8, HOGPD_REPORT);
                if(data[4] != 0) {
                    data[4] = 0;
                    app_hogpd_send_report(HID_KEYBOARD_REPORT_IDX, data, 8, HOGPD_REPORT);
                }
                print_uint32("Good payload: ", payload);
             } else {
                print_uint32("Err, got ", uart_rx_data >> 24);
             }
        }
    }
}

//////////////////////////////////////////////////////////////////////

static void uart1_tx_callback(uint16_t data_cnt)
{
}

//////////////////////////////////////////////////////////////////////

static void uart1_err_callback(uart_t *uart, uint8_t uart_err_status)
{
}

//////////////////////////////////////////////////////////////////////

void periph_init(void)
{
#if defined(__DA14531__)
    // In Boost mode enable the DCDC converter to supply VBAT_HIGH for the used GPIOs
    syscntl_dcdc_turn_on_in_boost(SYSCNTL_DCDC_LEVEL_3V0);
#else
    // Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while(!(GetWord16(SYS_STAT_REG) & PER_IS_UP))
        ;
    SetBits16(CLK_16M_REG, XTAL16_BIAS_SH_ENABLE, 1);
#endif

    // ROM patch
    patch_func();

    // Initialize peripherals
#if defined(CFG_PRINTF_UART2)
    uart_initialize(UART2, &uart_cfg);
#endif

    uart_initialize(UART1, &uart1_cfg);
    uart_receive(UART1, &uart1_buffer, 1, UART_OP_INTR);

    // Set pad functionality
    set_pad_functions();

    // Enable the pads
    GPIO_set_pad_latch_en(true);
}
