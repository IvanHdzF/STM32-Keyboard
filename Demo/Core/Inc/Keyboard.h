/*
 * Keyboard.h
 *
 *  Created on: Jan 23, 2024
 *      Author: rjmendez
 */

#ifndef APPLICATION_USER_INC_KEYBOARD_H_
#define APPLICATION_USER_INC_KEYBOARD_H_

void keyboard_Init(UART_HandleTypeDef *huart);

extern uint32_t eventFlags;

#endif /* APPLICATION_USER_INC_KEYBOARD_H_ */
