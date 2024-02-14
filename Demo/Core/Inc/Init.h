/*
 * Init.h
 *
 *  Created on: Jan 30, 2024
 *      Author: smoreno
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_
#include "services.h"



#define PERIPHERAL_PUBLIC_ADDRESS             {0xBC, 0xFC, 0x00, 0xE1, 0x80, 0x02}












/*Function Prototypes*/
void Init();
uint8_t Configure_HidPeripheral(void);

uint8_t hidSetTxPower(uint8_t level);
uint8_t hidDevice_Init(uint8_t IO_Capability, connParam_Type connParam,uint8_t dev_name_len, uint8_t *dev_name, uint8_t *addr);

/**
  * @brief Configures the device for LE Security Mode 1 and either Security Level 2 or 3
  * To set the Security Level 2 the MITM mode param shall be equal to FALSE, all the other params are ignored.
  * To set the Security Level 3 the MITM mode param shall be equal to TRUE, the other params shall be used to
  * signal if the device uses a fixed PIN or not.
  * @param MITM_Mode MITM mode. TRUE to set the security level 3, FALSE to set the security level 2
  * @param fixedPinUsed TRUE to use a fixed PIN, FALSE otherwise. If the MITM_Mode param is FALSE, this param is ignored
  * @param fixedPinValue PIN value
  * @retval Status of the call.
  */

uint8_t hidSetDeviceSecurty(uint8_t MITM_Mode, uint8_t fixedPinUsed, uint32_t fixedPinValue);

#endif /* INC_INIT_H_ */
