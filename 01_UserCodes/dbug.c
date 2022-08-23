/*
 * dbug.c
 *
 *  Created on: 5 июн. 2022 г.
 *      Author: me
 */

#include "dbug.h"

#ifdef DBUG_HUART

#include "main.h"

extern UART_HandleTypeDef DBUG_HUART;

static uint8_t out[2];

int __io_putchar(int ch){
	out[0]=ch;
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART1 and Loop until the end of transmission */
	HAL_UART_Transmit(&DBUG_HUART, (uint8_t*) &out[0], 1, 1);

	return ch;
}

#endif
