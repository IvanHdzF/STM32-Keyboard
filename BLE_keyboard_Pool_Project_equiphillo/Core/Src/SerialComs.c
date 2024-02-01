/*
 * SerialCom.c
 *
 *  Created on: Nov 16, 2023
 *      Author: rjmendez
 */

#include "SerialComs.h"
#include "string.h"
#include "stdio.h"
#include "keyboard.h"


UART_HandleTypeDef *comms;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint32_t rx_index = 0;
uint8_t receivedData = 0;

void SerialComInit(UART_HandleTypeDef *huart)
{
	comms = huart;

	if (HAL_UART_Receive_IT(huart, rx_buffer, 1) != HAL_OK) {
		// Handle error (e.g., print an error message or blink an LED)

		while(1){}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == comms->Instance)
	{
		receivedData = rx_buffer[rx_index];
		// Receive and display data from USART
		if(receivedData != '\n')
		{
			processInputData(&receivedData, sizeof(receivedData));
			HAL_UART_Transmit(comms, &receivedData, 1, HAL_MAX_DELAY);

		}
		else
		{
			printf("\n");
		}

		// Sum index value or set it to 0
		rx_index = (rx_index <= RX_BUFFER_SIZE) ? rx_index + 1 : 0;

		// Start a new reception
		HAL_UART_Receive_IT(comms, &rx_buffer[rx_index], 1);
	}
}
