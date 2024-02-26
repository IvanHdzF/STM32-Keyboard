/*
 * StateMachine.h
 *
 *  Created on: Jan 25, 2024
 *      Author: rjmendez
 */
#include "cmsis_os2.h"

#ifndef APPLICATION_USER_INC_STATEMACHINE_H_
#define APPLICATION_USER_INC_STATEMACHINE_H_

extern osEventFlagsId_t flagsId;

extern void SM_Init(UART_HandleTypeDef *huart);

//// Define state-specific action functions
//extern void handleSM_NORMAL(void);
//
//extern void handleSM_IDLE(void);
//
//extern void handleSM_SLEEP(void);

// Define events
typedef enum
{
	SM_NORMAL,
	SM_IDLE,
	SM_SLEEP
} SM_Status;

extern SM_Status currentState;

// Define events
typedef enum {
    EVENT_QUEUE_NOT_EMPTY,
	EVENT_QUEUE_EMPTY,
    EVENT_IDLE_CYCLE_COUNTER_REACHED
} Event;

extern SM_Status nextState[3][3];
extern void (*stateActions[3])(void);


// Main state machine execution loop
extern void runStateMachine(void *argument);

#endif /* APPLICATION_USER_INC_STATEMACHINE_H_ */
