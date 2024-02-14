/*
 * StateMachine.c
 *
 *  Created on: Jan 25, 2024
 *      Author: rjmendez
 */
#include "main.h"
#include "StateMachine.h"
#include "cmsis_os2.h"
#include "Keyboard.h"

UART_HandleTypeDef *SM_uart_Port;

SM_Status currentState;
osEventFlagsId_t flagsId;

osThreadId_t threadId;

Event flags;

// Define state-specific action functions
void handleSM_NORMAL(void);
void handleSM_IDLE(void);
void handleSM_SLEEP(void);

SM_Status nextState[3][3]= {
		{SM_NORMAL, SM_IDLE},
		{SM_NORMAL,SM_IDLE,SM_SLEEP},
		{SM_NORMAL}
};

void (*stateActions[3])(void) = {
    handleSM_NORMAL,
    handleSM_IDLE,
    handleSM_SLEEP
};

void SM_Init(UART_HandleTypeDef *huart)
{

	SM_uart_Port = huart;
	currentState = SM_NORMAL;

	if(osThreadNew(runStateMachine, NULL, NULL) == NULL )
	{
		while(1);
	}

	// Set event flags
	flagsId = osEventFlagsNew(NULL);
	if(flagsId == NULL)
	{
		while(1);
	}

}

// Main state machine execution loop
void runStateMachine(void *argument) {

	while(1){
		osEventFlagsWait(flagsId, 0x11U, osFlagsWaitAny, osWaitForever);
		osDelay(10U);
		Event event;
		osDelay(10U);

		//HAL_UART_Transmit(SM_uart_Port, setMssg, sizeof(setMssg), HAL_MAX_DELAY);
		switch(eventFlags)
		{
			case 0x00000001U:
				event = SM_NORMAL;
				break;
			case 0x00000010U:
				event = SM_IDLE;
				break;
			case 0x00000100U:
				event = SM_SLEEP;
				break;
		}

		// Get the next state based on the current state and event
		SM_Status afterState = nextState[currentState][event];

		// Execute the action associated with the current state
		stateActions[currentState]();

		// Transition to the next state
		currentState = afterState;
	}
}

// Define state-specific action functions
void handleSM_NORMAL(void) {
    // Perform actions specific to the normal state
	//uint8_t mssg[] = "NORMAL STATE \t";
	//HAL_UART_Transmit(SM_uart_Port, mssg, sizeof(mssg), HAL_MAX_DELAY);
}

void handleSM_IDLE(void) {
    // Perform actions specific to the idle state
	//uint8_t mssg2[] = "IDLE STATE \t";
	//HAL_UART_Transmit(SM_uart_Port, mssg2, sizeof(mssg2), HAL_MAX_DELAY);
}

void handleSM_SLEEP(void) {
    // Perform actions specific to the sleep state
	uint8_t mssg3[] = "SLEEP STATE \t";
	HAL_UART_Transmit(SM_uart_Port, mssg3, sizeof(mssg3), HAL_MAX_DELAY);
}
