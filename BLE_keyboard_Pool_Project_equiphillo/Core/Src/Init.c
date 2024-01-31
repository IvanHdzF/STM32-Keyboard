/*
 * Init.c
 * Initialization of GATT and GAP Services.
 *  Created on: Jan 30, 2024
 *      Author: smoreno
 */
/*Includes*/
#include "stdint.h"

#include "ble_status.h"

#include "sm.h"
#include "callbacks.h"
#include "ble_types.h"
#include "services.h"
#include "Init.h"

/*Structures ---------------------------------------------------------------------*/

/*Global Variables ---------------------------------------------------------------*/


/* Private function prototypes ---------------------------------------------------*/

void Init()
{

	uint8_t ret;

	/*Initializing and Reseting HCI*/
	hci_init(APP_UserEvtRx, NULL);
	hci_reset();
	HAL_Delay(100);

	ret = Configure_HidPeripheral();
	if(ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("Configure_HidPeripheral failed \r\n");
	}
	Reset_DiscoveryContext();


}




uint8_t Configure_HidPeripheral(void)
{
	uint8_t ret;
	devInfService_Type devInf;
	connParam_Type connParam;
	uint8_t addr[] = PERIPHERAL_PUBLIC_ADDRESS;
	uint8_t dev_name[]={'S', 'T', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd'};
	// uint8_t local_name[]={AD_TYPE_COMPLETE_LOCAL_NAME,'S', 'T', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd'};
	/* HID Peripheral Init */
	connParam.interval_min = 0x10;
	connParam.interval_max = 0x14;
	connParam.slave_latency = 0x14;
	connParam.timeout_multiplier = 0xD2;
	ret = hidDevice_Init(IO_CAP_DISPLAY_ONLY, connParam, sizeof(dev_name), dev_name, addr);
	if (ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("Error in hidDevice_Init() 0x%02x\n", ret);
		return ret;
	}

	/* Set the HID Peripheral Security */
	ret = hidSetDeviceSecurty(TRUE, USE_FIXED_PIN_FOR_PAIRING, 123456);
	if (ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("Error in hidSetDeviceSecurty() 0x%02x\n", ret);
		return ret;
	}


	/* Set the TX power -2 dBm */
	ret = hidSetTxPower(4);

	if (ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("Error in hidSetTxPower() 0x%02x\n", ret);
		return ret;
	}

	/**** Setup the GATT Database ****/

	/* Device Information Service:
	   NOTE: memcpy length parameter must be equal to defined macro parameter length
	   on hid_peripheral_config.h file */
	memcpy(devInf.manufacName, "ST Micro ", MANUFAC_NAME_LEN);
	memcpy(devInf.modelNumber, "0001", MODEL_NUMB_LEN);
	memcpy(devInf.fwRevision, "0630", FW_REV_LEN);
	memcpy(devInf.swRevision, "0001", SW_REV_LEN);
	devInf.pnpID[0] = 0x01;
	devInf.pnpID[1] = 0x30;
	devInf.pnpID[2] = 0x00;
	devInf.pnpID[3] = 0xfc;
	devInf.pnpID[4] = 0x00;
	devInf.pnpID[5] = 0xec;
	devInf.pnpID[6] = 0x00;


	ret = hidAddServices(&devInf);
	if (ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("Error in hidAddServices() 0x%02x\n", ret);
		return ret;
	}

	/* Set the HID Peripheral device discoverable */
	/*  ret = hidSetDeviceDiscoverable(LIMITED_DISCOVERABLE_MODE, sizeof(local_name), local_name);
	  if (ret != BLE_STATUS_SUCCESS) {
		BLUENRG_PRINTF("Error in hidSetDeviceDiscoverable() 0x%02x\n", ret);
		return ret;
	  }
	  */

	BLUENRG_PRINTF("HID Keyboard Configured\r\n");
	return BLE_STATUS_SUCCESS;
}



uint8_t hidSetDeviceSecurty(uint8_t MITM_Mode, uint8_t fixedPinUsed, uint32_t fixedPinValue)
{
  if (!MITM_Mode)
  {
    return -1;
  }
  uint8_t ret;
  ret = aci_gap_set_authentication_requirement(BONDING,
                                               //MITM_PROTECTION_REQUIRED,
                                              MITM_PROTECTION_NOT_REQUIRED,
                                              SC_IS_NOT_SUPPORTED,
                                               KEYPRESS_IS_NOT_SUPPORTED,
                                               7,
                                               16,
                                               fixedPinUsed,
                                               fixedPinValue,
                                               0x00);
  if(ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("aci_gap_set_authentication_requirement()failed: 0x%02x\r\n", ret);
  }
  else
    BLUENRG_PRINTF("aci_gap_set_authentication_requirement() --> SUCCESS\r\n");
  return ret;
}


uint8_t hidSetTxPower(uint8_t level)
{
  uint8_t ret;
  ret = aci_hal_set_tx_power_level(1, level);
  if (ret != BLE_STATUS_SUCCESS)
  {
    BLUENRG_PRINTF("Error in aci_hal_set_tx_power_level() 0x%04x\r\n", ret);
  }
  return ret;
}



/**
 * @brief This function sets public Address, Initializes Gatt and Gap Services according to HID BLE Keyboard specs.
 * @param IO_Capability set IO Capability
 * @param connParam Update values for Preferred Connection Parameters Characteristics
 * @param dev_name_len Device name Len
 * @param dev_name Device name buffer
 * @param addr  Numeric Addres
 * @retval BLE Status
 */
uint8_t hidDevice_Init(uint8_t IO_Capability, connParam_Type connParam,
					   uint8_t dev_name_len, uint8_t *dev_name, uint8_t *addr)
{
	uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	uint8_t ret;

	/* Set the device public address */
	ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, addr);
	if (ret != BLE_STATUS_SUCCESS)
	{
		// BLUENRG_PRINTF("Setting BD_ADDR failed 0x%02x\r\n", ret);
		return ret;
	}
	else
	{
		BLUENRG_PRINTF("Public address: ");
		for (uint8_t i = 5; i > 0; i--)
		{
			BLUENRG_PRINTF("%02X-", addr[i]);
		}
		BLUENRG_PRINTF("%02X\r\n", addr[0]);
	}

	/* GATT Init */
	ret = aci_gatt_init();
	if (ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("aci_gatt_init() failed: 0x%02x\r\n", ret);
		return ret;
	}
	else
		BLUENRG_PRINTF("aci_gatt_init() --> SUCCESS\r\n");

	/* GAP Init */
	uint8_t appareance[] = {0xC1, 0x03};

	ret = aci_gap_init(GAP_CENTRAL_ROLE | GAP_PERIPHERAL_ROLE, 0x0, 0x20, &service_handle,
					   &dev_name_char_handle, &appearance_char_handle);
	aci_gatt_update_char_value(service_handle, appearance_char_handle, 0, 2, appareance);

	ret = aci_gatt_update_char_value_ext(0, service_handle, dev_name_char_handle, 0, dev_name_len, 0, dev_name_len, dev_name);
	ret = aci_gatt_update_char_value_ext(0, service_handle, appearance_char_handle + 2, 0, sizeof(connParam), 0, sizeof(connParam), (uint8_t *)&connParam);

	/* Set IO capability */
	ret = aci_gap_set_io_capability(IO_Capability);
	if (ret != BLE_STATUS_SUCCESS)
	{
		BLUENRG_PRINTF("GAP_set_io failed: 0x%02x\r\n", ret);
	}
	return ret;
}
