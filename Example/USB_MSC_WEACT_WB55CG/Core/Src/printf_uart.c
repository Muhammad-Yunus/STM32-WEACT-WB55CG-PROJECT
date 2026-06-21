/*
 * printf_uart.c
 *
 *  Created on: Jun 17, 2026
 *      Author: Asus
 */


#include "main.h"
#include <stdint.h>

extern UART_HandleTypeDef huart1;


int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
