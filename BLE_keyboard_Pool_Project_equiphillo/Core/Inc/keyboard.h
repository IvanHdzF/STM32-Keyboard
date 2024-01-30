/*
 * keyboard.h
 *
 *  Created on: Jan 25, 2024
 *      Author: smoreno
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_


#define HID_DEVICE_READY_TO_NOTIFY (0x04)
#define NUM_0      0x30
#define NUM_9      0x39
#define CHAR_A     0x41
#define CHAR_Z     0x5A
#define CHAR_a     0x61
#define CHAR_z     0x7A
#define RETURN     0x0D
#define BACKSPACE  0x08
#define TAB        0x09
#define SPACE      0x20
#define IDLE_CONNECTION_TIMEOUT (120*1000) // 2 min
#define KEY_TABLE_LEN 33





void processInputData(uint8_t* data_buffer, uint8_t Nb_bytes);
/**
 * @brief Returns the device status
 * @retval The device status. Possible values are:
 * - HID_DEVICE_READY_TO_NOTIFY: device connected, bonded and paired
 * - HID_DEVICE_READY_TO_POWER_SAVE: device in idle and not notification pending
 * - HID_DEVICE_BONDED: device bonded and not connected
 * - HID_DEVICE_CONNECTED: device connected and not bonded
 * - HID_DEVICE_BUSY: device busy executing a state machine
 * - HID_DEVICE_NOT_CONNECTED: device not connected and not bonded
 */
uint8_t hidDeviceStatus(void);
uint8_t hid_keyboard_map(uint8_t charac, uint8_t *upperCase);
uint8_t hidSendReport(uint8_t id, uint8_t type, uint8_t reportLen, uint8_t *reportData);



#endif /* INC_KEYBOARD_H_ */
