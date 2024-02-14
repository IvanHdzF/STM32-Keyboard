/*
 * sm.c
 *
 *  Created on: Jan 30, 2024
 *      Author: smoreno
 */

#include "sm.h"
#include "stdint.h"
#include "bluenrg1_gap.h"
#include "services.h"
#include "callbacks.h"
#include "kbd_process.h"
#include "Keyboard.h"
#include "cmsis_os2.h"





volatile int app_flags = SET_CONNECTABLE;

uint16_t connection_handle=0;
uint8_t protocolMode = 0;
uint16_t discovery_time     = 3000;
uint8_t  mtu_exchanged      = 0;
uint8_t  mtu_exchanged_wait = 0;
uint8_t protocolModeSet = 0;
uint16_t write_char_len;
extern uint16_t hid_service_handle, protocol_mode_char_handle;
extern uint16_t device_role;
extern discoveryContext_t discovery;
extern uint8_t advtServUUID[100];
extern uint8_t local_name[];
extern uint8_t receivedData;

uint8_t Local_Name_Length = 15;

void BLE_Process(void)
{

	//uint8_t data[] = "s";




	hci_user_evt_proc();

	if (APP_FLAG(SET_CONNECTABLE))
	{
		Connection_StateMachine();
	}

	if (APP_FLAG(CONNECTED) && !protocolModeSet)
	{
		protocolMode = REPORT_PROTOCOL_MODE;
		BLUENRG_PRINTF("protocol Mode was set to 1\r\n");
		aci_gatt_update_char_value(hid_service_handle, protocol_mode_char_handle, 0, 1, &protocolMode);
		protocolModeSet = 1;
	}
	if (APP_FLAG(CONNECTED))
	{
		if ((mtu_exchanged == 0) && (mtu_exchanged_wait == 0))
		{
			BLUENRG_PRINTF("ROLE SLAVE (mtu_exchanged %d, mtu_exchanged_wait %d)\r\n",
				   mtu_exchanged, mtu_exchanged_wait);

			mtu_exchanged_wait = 1;
			uint8_t ret = aci_gatt_exchange_config(connection_handle);
			if (ret != BLE_STATUS_SUCCESS)
			{
				BLUENRG_PRINTF("aci_gatt_exchange_configuration error 0x%02x\r\n", ret);
			}
		}
	}
	if (APP_FLAG(CONNECTED) && APP_FLAG(NOTIFICATIONS_ENABLED))
	{
		BLUENRG_PRINTF("Trying to print smth\r\n");
		//status = osMessageQueueGet(mssgQ, &rx_buffer[rx_index], 0, 0);
		//processInputData(data, sizeof(data));
		//HAL_GPIO_TogglePin(GPIOA, LED_Pin);
		//osDelay(1000);
	}
}



void Connection_StateMachine(void)
{
    uint8_t ret;

    switch (discovery.device_state)
    {
    case (INIT_STATE):
    {
        BLUENRG_PRINTF("INSIDE INIT STATE\r\n");
        Reset_DiscoveryContext();
        discovery.device_state = START_DISCOVERY_PROC;
    }
    break; /* end case (INIT_STATE) */
    case (START_DISCOVERY_PROC):
    {
      //  BLUENRG_PRINTF("INSIDE START_DISCOVERY_PROC\r\n");

        /*
        ADV_IND,
        (ADV_INTERVAL_MIN_MS*1000)/625,
        (ADV_INTERVAL_MAX_MS*1000)/625,
        */


        ret = aci_gap_start_general_discovery_proc((ADV_INTERVAL_MIN_MS * 1000) / 625,
            (ADV_INTERVAL_MAX_MS * 1000) / 625,
            PUBLIC_ADDR,
            0x00);
        if (ret != BLE_STATUS_SUCCESS)
        {
            BLUENRG_PRINTF("aci_gap_start_limited_discovery_proc() failed: %02X\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            BLUENRG_PRINTF("aci_gap_start_limited_discovery_proc OK\r\n");
            discovery.startTime = HAL_GetTick();
            discovery.check_disc_proc_timer = TRUE;
            discovery.check_disc_mode_timer = FALSE;
            discovery.device_state = WAIT_TIMER_EXPIRED;
        }
    }
    break;/* end case (START_DISCOVERY_PROC) */
    case (WAIT_TIMER_EXPIRED):
    {
        /* Verify if startTime check has to be done  since discovery procedure is ongoing */
        if (discovery.check_disc_proc_timer == TRUE)
        {
            /* check startTime value */
            if (HAL_GetTick() - discovery.startTime > discovery_time)
            {
                discovery.check_disc_proc_timer = FALSE;
                discovery.startTime = 0;
                discovery.device_state = DO_TERMINATE_GAP_PROC;
            }/* if (HAL_GetTick() - discovery.startTime > discovery_time) */
        }
        /* Verify if startTime check has to be done  since discovery mode is ongoing */
        else if (discovery.check_disc_mode_timer == TRUE)
        {
            /* check startTime value */
            if (HAL_GetTick() - discovery.startTime > discovery_time)
            {
                discovery.check_disc_mode_timer = FALSE;
                discovery.startTime = 0;

                /* Discovery mode is ongoing: set non discoverable mode */
                discovery.device_state = DO_NON_DISCOVERABLE_MODE;

            }/* else if (discovery.check_disc_mode_timer == TRUE) */
        }/* if ((discovery.check_disc_proc_timer == TRUE) */
    }
    break; /* end case (WAIT_TIMER_EXPIRED) */

    case (DO_NON_DISCOVERABLE_MODE):
    {
       // BLUENRG_PRINTF("INSIDE DO_NON_DISCOVERABLE_MODE\r\n");
        ret = aci_gap_set_non_discoverable();
        if (ret != BLE_STATUS_SUCCESS)
        {
            BLUENRG_PRINTF("aci_gap_set_non_discoverable() failed: 0x%02x\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            BLUENRG_PRINTF("aci_gap_set_non_discoverable() OK\r\n");
            /* Restart Central discovery procedure */
            discovery.device_state = INIT_STATE;
        }
    }
    break; /* end case (DO_NON_DISCOVERABLE_MODE) */
    case (DO_TERMINATE_GAP_PROC):
    {
       // BLUENRG_PRINTF("INSIDE DO_TERMINATE_GAP_PROC\r\n");
        /* terminate gap procedure */
        ret = aci_gap_terminate_gap_proc(0x02); // GENERAL_DISCOVERY_PROCEDURE
        if (ret != BLE_STATUS_SUCCESS)
        {
            BLUENRG_PRINTF("aci_gap_terminate_gap_procedure() failed: 0x%02x\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
            break;
        }
        else
        {
            BLUENRG_PRINTF("aci_gap_terminate_gap_procedure() OK\r\n");
            discovery.device_state = WAIT_EVENT; /* wait for GAP procedure complete */
        }
    }
    break; /* end case (DO_TERMINATE_GAP_PROC) */
    case (DO_DIRECT_CONNECTION_PROC):
    {
      //  BLUENRG_PRINTF("INSIDE DO_DIRECT_CONNECTION_PROC\r\n");
        BLUENRG_PRINTF("Device Found with address: ");
        for (uint8_t i = 5; i > 0; i--)
        {
            BLUENRG_PRINTF("%02X-", discovery.device_found_address[i]);
        }
        BLUENRG_PRINTF("%02X\r\n", discovery.device_found_address[0]);
        /* Do connection with first discovered device */
        ret = aci_gap_create_connection(SCAN_P, SCAN_L,
            discovery.device_found_address_type, discovery.device_found_address,
            PUBLIC_ADDR, 40, 40, 0, 60, 2000, 2000);
        if (ret != BLE_STATUS_SUCCESS)
        {
            BLUENRG_PRINTF("aci_gap_create_connection() failed: 0x%02x\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            BLUENRG_PRINTF("aci_gap_create_connection() OK\r\n");
            discovery.device_state = WAIT_EVENT;
        }
    }
    break; /* end case (DO_DIRECT_CONNECTION_PROC) */
    case (WAIT_EVENT):
    {
    //    BLUENRG_PRINTF("INSIDE WAIT_EVENT\r\n");
        discovery.device_state = WAIT_EVENT;
    }
    break; /* end case (WAIT_EVENT) */



    case (ENTER_DISCOVERY_MODE):
    {
    //    BLUENRG_PRINTF("INSIDE ENTER_DISCOVERY_MODE\r\n");
        /* Put Peripheral device in discoverable mode */

        /* disable scan response */
        hci_le_set_scan_response_data(0, NULL);

        /*CHANGEEEDDDDDDDDDDDDDDDDDDDDDDDDDDD**/
        ret = hidSetDeviceDiscoverable(LIMITED_DISCOVERABLE_MODE, Local_Name_Length, local_name);
        /*ret = aci_gap_set_discoverable(ADV_DATA_TYPE, ADV_INTERV_MIN, ADV_INTERV_MAX, PUBLIC_ADDR,
            NO_WHITE_LIST_USE, sizeof(local_name), local_name, 0, NULL,
            0x0, 0x0);*/


        if (ret != BLE_STATUS_SUCCESS) {
            BLUENRG_PRINTF("Error in hidSetDeviceDiscoverable() 0x%02x\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            BLUENRG_PRINTF("aci_gap_set_discoverable() OK\r\n");
            discovery.startTime = HAL_GetTick();
            discovery.check_disc_mode_timer = TRUE;
            discovery.check_disc_proc_timer = FALSE;
            discovery.device_state = WAIT_TIMER_EXPIRED;
        }
    }
    break; /* end case (ENTER_DISCOVERY_MODE) */
    case (DISCOVERY_ERROR):
    {
     //   BLUENRG_PRINTF("INSIDE DISCOVERY_ERROR\r\n");
        Reset_DiscoveryContext();
    }
    break; /* end case (DISCOVERY_ERROR) */
    default:
        break;
    }/* end switch */

}/* end Connection_StateMachine() */




uint8_t hidSetDeviceDiscoverable(uint8_t mode, uint8_t nameLen, uint8_t *name)
{
  uint8_t ret;
  advtServUUID[0] = AD_TYPE_16_BIT_SERV_UUID;

  // advtServUUID[1]=HUMAN_INTERFACE_DEVICE_SERVICE_UUID;
  // advtServUUID[2]=BATTERY_SERVICE_SERVICE_UUID;
  // advtServUUID[3]=DEVICE_INFORMATION_SERVICE_UUID;
  // advtServUUID[100]=0x1111;

  BLE_Profile_Add_Advertisment_Service_UUID(HUMAN_INTERFACE_DEVICE_SERVICE_UUID);
  BLE_Profile_Add_Advertisment_Service_UUID(BATTERY_SERVICE_SERVICE_UUID);
  BLE_Profile_Add_Advertisment_Service_UUID(DEVICE_INFORMATION_SERVICE_UUID);
  if (mode == LIMITED_DISCOVERABLE_MODE)
  {
    BLUENRG_PRINTF("Set limited Discoverable Mode.\n");
    ret = aci_gap_set_limited_discoverable(ADV_IND,
                                           (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                           PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                           nameLen, name, 0, NULL, 0, 0);
    if (ret != BLE_STATUS_SUCCESS)
    {
      BLUENRG_PRINTF("aci_gap_set_discoverable() failed: 0x%02x\r\n", ret);
    }
    else
    {
      BLUENRG_PRINTF("aci_gap_set_discoverable() LIMITED --> SUCCESS\r\n");
    }
    return ret;
  }
  else if (mode == GENERAL_DISCOVERABLE_MODE)
  {
    // BLUENRG_PRINTF("Set General Discoverable Mode.\n");
    ret = aci_gap_set_discoverable(ADV_IND,
                                   (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                                   STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                   nameLen, name, 0, NULL, 0, 0);
    if (ret != BLE_STATUS_SUCCESS)
    {
      // BLUENRG_PRINTF("aci_gap_set_discoverable() failed: 0x%02x\r\n",ret);
    }
    else
    {
      BLUENRG_PRINTF("aci_gap_set_discoverable() GENERAL--> SUCCESS\r\n");
    }

    return ret;
  }
  BLUENRG_PRINTF("Wrong mode, device couldn't set as discoverable.\n");
  return -1;
}
void receiveData(uint8_t *data_buffer, uint8_t Nb_bytes)
{
  BLUENRG_PRINTF("Received: ");
  for (int i = 0; i < Nb_bytes; i++)
  {
    BLUENRG_PRINTF("%c", data_buffer[i]);
  }
  BLUENRG_PRINTF("\r\n");
  fflush(stdout);
}

void Reset_DiscoveryContext(void)
{
    discovery.check_disc_proc_timer = FALSE;
    discovery.check_disc_mode_timer = FALSE;
    discovery.is_device_found = FALSE;
    discovery.do_connect = FALSE;
    discovery.startTime = 0;
    discovery.device_state = INIT_STATE;
    BLUENRG_memset(&discovery.device_found_address[0], 0, 6);
    device_role = 0xFF;
    mtu_exchanged = 0;
    mtu_exchanged_wait = 0;
    write_char_len = CHAR_VALUE_LENGTH - 3;
}
