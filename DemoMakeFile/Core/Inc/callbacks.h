/*
 * callbacks.h
 *
 *  Created on: Jan 30, 2024
 *      Author: smoreno
 */

#ifndef INC_CALLBACKS_H_
#define INC_CALLBACKS_H_
#include "stdint.h"

typedef struct discoveryContext_s {
  uint8_t  check_disc_proc_timer;
  uint8_t  check_disc_mode_timer;
  uint8_t  is_device_found;
  uint8_t  do_connect;
  uint32_t startTime;
  uint8_t  device_found_address_type;
  uint8_t  device_found_address[6];
  uint16_t device_state;
} discoveryContext_t;

/*Function Prototypes*/
void    APP_UserEvtRx(void *pData);
uint8_t Find_DeviceName(uint8_t data_length, uint8_t *data_value);
void    Attribute_Modified_CB(uint16_t handle, uint8_t data_length,
                              uint8_t *att_data);

#endif /* INC_CALLBACKS_H_ */
