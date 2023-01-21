//////////////////////////////////////////////////////////////////////

#include "user_periph_setup.h"
#include "datasheet.h"
#include "system_library.h"
#include "rwip_config.h"
#include "gpio.h"
#include "uart.h"
#include "syscntl.h"
#include "arch_console.h"

void send_payload(int32 payload);

//////////////////////////////////////////////////////////////////////

static void uart1_err_callback(uart_t *uart, uint8_t uart_err_status);
static void uart1_tx_callback(uint16_t data_cnt);
static void uart1_rx_callback(uint16_t data_cnt);

//////////////////////////////////////////////////////////////////////

uint8_t uart1_buffer;
int uart_rx_byte_count;
uint8_t uart_rx_data[4];
uint8_t uart_tx_data[4];

// Configuration struct for UART1
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

static const uart_cfg_t uart_cfg = {
    .baud_rate = UART2_BAUDRATE,
    .data_bits = UART2_DATABITS,
    .parity = UART2_PARITY,
    .stop_bits = UART2_STOPBITS,
    .auto_flow_control = UART2_AFCE,
    .use_fifo = UART2_FIFO,
    .tx_fifo_tr_lvl = UART2_TX_FIFO_LEVEL,
    .rx_fifo_tr_lvl = UART2_RX_FIFO_LEVEL,
    .intr_priority = 2,
};
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

void print_uint32(uint32_t x)
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
    arch_printf(txt);
}

//////////////////////////////////////////////////////////////////////
// send a 21 bit message (32 bits with checksum and id bits)

void send_message(uint32 message)
{
    int d0 = message & 0x7f;
    int d1 = (message >> 7) & 0x7f;
    int d2 = (message >> 14) & 0x7f;
    uart_tx_data[0] = (d0 ^ 0x7f ^ d1 ^ d2 ^ 0x55) | 0x80;
    uart_tx_data[1] = d2;
    uart_tx_data[2] = d1;
    uart_tx_data[3] = d0;
    uart_send(UART1, uart_tx_data, 4, UART_OP_INTR);
}

//////////////////////////////////////////////////////////////////////

uint32 decode_message(byte data[4])
{
    int d2 = data[1];
    int d1 = data[2];
    int d0 = data[3];
    uint32_t checksum = d0 ^ 0x7f ^ d1 ^ d2 ^ 0x55;
    if((data[0] & 0x7f) == checksum) {
        return (d2 << 14) | (d1 << 7) | d0;
    }
    return 0xffffffff;
}

//////////////////////////////////////////////////////////////////////

static void uart1_rx_callback(uint16_t data_cnt)
{
    uint8_t got_byte = uart1_buffer;
    uart_receive(UART1, &uart1_buffer, 1, UART_OP_INTR);

    // if top bit is set, it's a checksum byte, grab it and reset stuff
    if((got_byte & 0x80) != 0) {
        uart_rx_byte_count = 0;
        uart_rx_data[0] = got_byte & 0x7f;
        arch_printf("Got checksum: ");
        print_uint32(got_byte & 0xff);
    } else {
        // if top bit is clear, it's a data byte, add it to the data and if got 3 check it
        uart_rx_byte_count += 1;
        uart_rx_data[uart_rx_byte_count] = got_byte & 0x7f;
        if(uart_rx_byte_count == 3) {
            uint32_t payload = decode_message(uart_rx_data);
            if(payload != 0xffffffff) {
                send_message(payload);
                GPIO_TOGGLE(GPIO_LED_PORT, GPIO_LED_PIN);
                arch_printf("Got message ");
                print_uint32(payload);
                send_payload(payload);
            } else {
                arch_printf("Err, expected ");
                print_uint32(uart_rx_data[0]);
            }
            uart_rx_byte_count = 0;
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
