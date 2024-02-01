/*
 * SerialCom.h
 *
 *  Created on: Nov 16, 2023
 *      Author: rjmendez
 */

#ifndef INC_SERIALCOM_H_
#define INC_SERIALCOM_H_

#include "main.h"

// Buffer to store received data
#define RX_BUFFER_SIZE 255

void SerialComInit(UART_HandleTypeDef *huart);

#endif /* INC_SERIALCOM_H_ */

