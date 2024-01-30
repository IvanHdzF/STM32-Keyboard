/*
 * app_bluenrg.h
 *
 *  Created on: Dec 7, 2021
 *      Author: hussamaldean
 */

#ifndef INC_APP_BLUENRG_H_
#define INC_APP_BLUENRG_H_



#include "stdint.h"
#include <stdint.h>
#include "app_bluenrg.h"

#include "bluenrg_conf.h"
#include "bluenrg1_types.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_hci_le.h"
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "keyboard.h"

#include "services.h"

void bluenrg_init(void);
void bluenrg_process(void);

/* protocol Mode */
#define BOOT_PROTOCOL_MODE          (0x00)
#define REPORT_PROTOCOL_MODE        (0x01)

/*for state machine, i guess*/

/**
  * @brief  Discovery States
 */
#define SET_CONNECTABLE           0x0100
#define CONNECTED                 0x0200
#define NOTIFICATIONS_ENABLED     0x0400
#define INIT_STATE                0x00
#define START_DISCOVERY_PROC      0x01
#define WAIT_EVENT                0x02
#define WAIT_TIMER_EXPIRED        0x04
#define DO_DIRECT_CONNECTION_PROC 0x08
#define ENTER_DISCOVERY_MODE      0x10
#define DO_TERMINATE_GAP_PROC     0x20
#define DO_NON_DISCOVERABLE_MODE  0x40
#define DISCOVERY_ERROR           0x80
 /*---------- Scan Interval: time interval from when the Controller started its last scan until it begins the subsequent scan (for a number N, Time = N x 0.625 msec) -----------*/
#define SCAN_P      16384
/*---------- Scan Window: amount of time for the duration of the LE scan (for a number N, Time = N x 0.625 msec) -----------*/
#define SCAN_L      16384

#define APP_FLAG(flag) (app_flags & flag)

#define APP_FLAG_SET(flag) (app_flags |= flag)
#define APP_FLAG_CLEAR(flag) (app_flags &= ~flag)

/* Added flags for handling TX, RX characteristics discovery */
#define START_READ_TX_CHAR_HANDLE 0x0800
#define END_READ_TX_CHAR_HANDLE   0x1000
#define START_READ_RX_CHAR_HANDLE 0x2000
#define END_READ_RX_CHAR_HANDLE   0x4000
/* GATT EVT_BLUE_GATT_TX_POOL_AVAILABLE event */
#define TX_BUFFER_FULL            0x8000
#define BLE_SAMPLE_APP_COMPLETE_LOCAL_NAME_SIZE  18

void Connection_StateMachine(void);
void Reset_DiscoveryContext(void);


void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t* att_data);

#endif /* INC_APP_BLUENRG_H_ */
