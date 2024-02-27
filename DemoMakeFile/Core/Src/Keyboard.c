/*
 * Keyboard.c
 *
 *  Created on: Jan 23, 2024
 *      Author: rjmendez
 */

#include "StateMachine.h"
#include "cmsis_os2.h"
#include "kbd_process.h"
#include "main.h"
#include "stdio.h"


#include "FreeRTOS.h"
#include "task.h"

// Buffer to store received data
#define RX_BUFFER_SIZE 100

// Message Queue creation variables
#define MAX_NUM_MESSAGES 100
#define MAX_MESSAGE_SIZE 1 // This size is in bytes

#define MAX_IDLE_CYCLES 400 // 40 equals a second

UART_HandleTypeDef *uart_Port;

osMessageQueueId_t mssgQ;

osTimerId_t mssgQ_timer;

osStatus_t status;

extern osThreadId_t threadId;

static uint8_t mssgQ_buffer[RX_BUFFER_SIZE];
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint32_t rx_index = 0;
static uint32_t idleCycles = 0;

uint32_t eventFlags = 0;

void gotoSleep();
void messageQ_TimerCallback();

void keyboard_Init(UART_HandleTypeDef *huart) {
  uart_Port = huart;

  mssgQ = osMessageQueueNew(MAX_NUM_MESSAGES, MAX_MESSAGE_SIZE, NULL);
  // In case MessageQueue fails
  if (mssgQ == NULL) {
    while (1)
      ;
  }

  mssgQ_timer = osTimerNew(messageQ_TimerCallback, osTimerPeriodic, NULL, NULL);
  // In case timer for MessageQueue fails
  if (mssgQ_timer == NULL) {
    while (1)
      ;
  }
  if (osTimerStart(mssgQ_timer, 25U) != osOK) {
    while (1)
      ;
  }

  if (HAL_UART_Receive_IT(huart, rx_buffer, 1) != HAL_OK) {
    while (1) {
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == uart_Port->Instance) {
    // Send data to queue
    status = osMessageQueuePut(mssgQ, &rx_buffer[rx_index], 0, 0);

    // Set flags for event
    eventFlags = osEventFlagsSet(flagsId, 0x00000001U);
    // osDelay(2U);

    // Start a new reception
    HAL_UART_Receive_IT(uart_Port, &rx_buffer[rx_index], 1);
  }
}

void messageQ_TimerCallback() {
  uint32_t messagesCount = 0;
  messagesCount = osMessageQueueGetCount(mssgQ);
  // Check if queue has data saved inside
  if (messagesCount > 0) {
    for (int i = 0; i < messagesCount; i++) {
      osMessageQueueGet(mssgQ, &mssgQ_buffer[i], 0, 0);
      // HAL_UART_Transmit(uart_Port, mssgQ_buffer, 1, HAL_MAX_DELAY);
    }
    taskENTER_CRITICAL();
    processInputData(mssgQ_buffer, 1);
    uint8_t data[] = {0x10};
    processInputData(data, sizeof(data));
    taskEXIT_CRITICAL();

    // Reset cycles to activate sleep mode
    idleCycles = 0;
  } else {
    // Set flag for event IDLE
    eventFlags = osEventFlagsSet(flagsId, 0x00000010U);
    idleCycles++;
  }

  if (idleCycles == MAX_IDLE_CYCLES) {
    gotoSleep();
  }
}

void gotoSleep() {
  uint8_t sleepMssg[] = "toi mimido";
  HAL_UART_Transmit(uart_Port, sleepMssg, sizeof(sleepMssg), HAL_MAX_DELAY);
  // Set flag for event SLEEP

  eventFlags = osEventFlagsSet(flagsId, 0x00000100U);
}
