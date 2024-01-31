/*
 * keyboard.c
 *
 *  Created on: Jan 25, 2024
 *      Author: smoreno
 */
#include "ble_status.h"
#include "ble_types.h"
#include "keyboard.h"
#include "services.h"


extern uint16_t hid_service_handle;
extern uint16_t input_report_char_handle;

typedef struct hidValueS {
  uint8_t key;
  uint8_t hid[2];
} hidValueType;

hidValueType lookupTable[KEY_TABLE_LEN] = {
  {0x21, {TRUE,  0x1E}},
  {0x22, {TRUE,  0x34}},
  {0x23, {TRUE,  0x20}},
  {0x24, {TRUE,  0x21}},
  {0x25, {TRUE,  0x22}},
  {0x26, {TRUE,  0x24}},
  {0x27, {FALSE, 0x34}},
  {0x28, {TRUE,  0x26}},
  {0x29, {TRUE,  0x27}},
  {0x2A, {TRUE,  0x25}},
  {0x2B, {TRUE,  0x2E}},
  {0x2C, {FALSE, 0x36}},
  {0x2D, {FALSE, 0x2D}},
  {0x2E, {FALSE, 0x37}},
  {0x2F, {FALSE, 0x38}},
  {0x3A, {TRUE,  0x33}},
  {0x3B, {FALSE, 0x33}},
  {0x3C, {TRUE,  0x36}},
  {0x3D, {FALSE, 0x2E}},
  {0x3E, {TRUE,  0x37}},
  {0x3F, {TRUE,  0x38}},
  {0x40, {TRUE,  0x1F}},
  {0x5B, {FALSE, 0x2F}},
  {0x5C, {FALSE, 0x31}},
  {0x5D, {FALSE, 0x30}},
  {0x5E, {TRUE,  0x23}},
  {0x5F, {TRUE,  0x2D}},
  {0x60, {FALSE, 0x35}},
  {0x7B, {TRUE,  0x2F}},
  {0x7C, {TRUE,  0x31}},
  {0x7D, {TRUE,  0x30}},
  {0x7E, {TRUE,  0x35}},
  {0x7F, {FALSE, 0x4C}},
};


void processInputData(uint8_t* data_buffer, uint8_t Nb_bytes)
{
  uint8_t ret, i, upperCase, nmbTimes, keys[8]={0,0,0,0,0,0,0,0};
  //Check if device is connected, bonded and paired
  uint8_t offset=2;
  if (HID_DEVICE_READY_TO_NOTIFY) {
    for (i=0; i<Nb_bytes; i++) {
      keys[offset] = hid_keyboard_map(data_buffer[i], &upperCase);
      if (upperCase)
    keys[0] = 0x02;
      else
    keys[0] = 0x00;
      ret = hidSendReport(0, INPUT_REPORT, sizeof(keys), keys);
      if (ret != BLE_STATUS_SUCCESS)
        printf("Error during send the report %02x\n", ret);
      keys[0] = 0;
      keys[offset] = 0;
      nmbTimes = 0;
      do {
    ret = hidSendReport(0, INPUT_REPORT, sizeof(keys), keys);
        nmbTimes++;
      } while ((ret != BLE_STATUS_SUCCESS) && (nmbTimes < 200));
    }
  }
}



uint8_t hid_keyboard_map(uint8_t charac, uint8_t *upperCase)
{
  uint8_t hidValue, i;

  hidValue = 0;
  *upperCase = FALSE;


  //Handles numbers (0-9)
  if ((charac >= NUM_0) && (charac <= NUM_9)) {
    hidValue = charac - 0x30; //0x30 = 48 dec = 0 ASCII
    if (hidValue == 0)
      hidValue = 0x27; //0x27 = 39 dec = ' ASCII
    else
      hidValue += 0x1D; //0x1D = 29 dec = [GROUP SEPARATOR] ASCII
  }


  //Handles upper cases (A-Z, 0x41-0x5A)
  if ((charac >= CHAR_A)  && (charac <= CHAR_Z)) {
    hidValue = charac - 0x41 + 0x04;
    *upperCase = TRUE;
  }

  //Handles down cases (a-z, 0x61-0x7A)
  if ((charac >= CHAR_a)  && (charac <= CHAR_z)) {
    hidValue = charac - 0x61 + 0x04;
  }
  //
  else
  {
    for (i=0; i<KEY_TABLE_LEN; i++) {
      if (lookupTable[i].key == charac) {
        *upperCase = lookupTable[i].hid[0];
        hidValue = lookupTable[i].hid[1];
        break;
      }
    }
  }

  switch(charac) {
  case RETURN:
    hidValue = 0x28;
    break;
  case BACKSPACE:
    hidValue = 0x02A;
    break;
  case SPACE:
    hidValue = 0x2C;
    break;
  case TAB:
    hidValue = 0x2B;
    break;
  }

  return hidValue;
}


uint8_t hidSendReport(uint8_t id, uint8_t type, uint8_t reportLen, uint8_t *reportData)
{
	uint8_t ret = aci_gatt_update_char_value(hid_service_handle,input_report_char_handle,0,reportLen,reportData);
	if (ret==BLE_STATUS_SUCCESS)
	{
		printf("Si se pudo!!!!! :) \r\n");
	}
    return ret;
}
