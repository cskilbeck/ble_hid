//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#include "gpio.h"
#include "uart.h"
#include "spi.h"
#include "spi_flash.h"
#include "i2c.h"
#include "i2c_eeprom.h"
#include "../../../common/int_types.h"

//////////////////////////////////////////////////////////////////////

#define GPIO_DATA_REG(port) (*(volatile uint16_t *)(GPIO_BASE + (port << 5)))
#define GPIO_SET_DATA_REG(port) (*(volatile uint16_t *)(GPIO_BASE + (port << 5) + 2))
#define GPIO_CLR_DATA_REG(port) (*(volatile uint16_t *)(GPIO_BASE + (port << 5) + 4))
#define GPIO_MODE_REG(port, pin) (*(volatile uint16_t)(GPIO_BASE + (port << 5) + 6 + (pin << 1)))

#define GPIO_SET(port, pin) (GPIO_SET_DATA_REG(port) = (1 << pin))
#define GPIO_CLEAR(port, pin) (GPIO_CLR_DATA_REG(port) = (1 << pin))
#define GPIO_TOGGLE(port, pin) (GPIO_DATA_REG(port) ^= (1 << pin))

//////////////////////////////////////////////////////////////////////
// GPIO TYPE    FUNCTION
//
// 0    B       RESET
// 1    X       NA
// 2    B       ? VBAT
// 3    X       NA
// 4    X       NA
// 5    B       ? WAKEUP / UART2_TX
// 6    A       UART1_TX
// 7    A       ? BATT_CHG
// 8    A       UART1_RX
// 9    A       DEBUG_LED / SWDIO
// 10   A       SWCLK
// 11   A       ? WAKEUP / DEBUG_BUTTON

//////////////////////////////////////////////////////////////////////

#define UART2_TX_PORT GPIO_PORT_0
#define UART1_TX_PORT GPIO_PORT_0
#define UART1_RX_PORT GPIO_PORT_0

#define UART2_TX_PIN GPIO_PIN_5
#define UART1_TX_PIN GPIO_PIN_6
#define UART1_RX_PIN GPIO_PIN_8

#define DEBUGGING

#define UART2_BAUDRATE UART_BAUDRATE_115200
#define UART2_DATABITS UART_DATABITS_8
#define UART2_PARITY UART_PARITY_NONE
#define UART2_STOPBITS UART_STOPBITS_1
#define UART2_AFCE UART_AFCE_DIS
#define UART2_FIFO UART_FIFO_EN
#define UART2_TX_FIFO_LEVEL UART_TX_FIFO_LEVEL_0
#define UART2_RX_FIFO_LEVEL UART_RX_FIFO_LEVEL_0

#define UART1_BAUDRATE UART_BAUDRATE_1000000
#define UART1_DATABITS UART_DATABITS_8
#define UART1_PARITY UART_PARITY_NONE
#define UART1_STOPBITS UART_STOPBITS_1
#define UART1_AFCE UART_AFCE_DIS
#define UART1_FIFO UART_FIFO_EN
#define UART1_TX_FIFO_LEVEL UART_TX_FIFO_LEVEL_0
#define UART1_RX_FIFO_LEVEL UART_RX_FIFO_LEVEL_0

#define GPIO_LED_PORT GPIO_PORT_0
#define GPIO_LED_PIN GPIO_PIN_9

#define SPI_EN_PORT GPIO_PORT_0
#define SPI_EN_PIN GPIO_PIN_3

#define SPI_CLK_PORT GPIO_PORT_0
#define SPI_CLK_PIN GPIO_PIN_4

#define SPI_DO_PORT GPIO_PORT_0
#define SPI_DO_PIN GPIO_PIN_0

#define SPI_DI_PORT GPIO_PORT_0
#define SPI_DI_PIN GPIO_PIN_3

//////////////////////////////////////////////////////////////////////

#if DEVELOPMENT_DEBUG
void GPIO_reservations(void);
#endif

void set_pad_functions(void);
void periph_init(void);
void print_uint32(char const *msg, uint32_t x);


