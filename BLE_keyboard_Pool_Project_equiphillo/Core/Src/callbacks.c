/*
 * callbacks.c
 *
 *  Created on: Jan 30, 2024
 *      Author: smoreno
 */
#include "callbacks.h"
#include "hci_const.h"
#include "stdint.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_types.h"
#include "sm.h"
#include "services.h"
#include "main.h"


/*Global Variables*/

uint8_t advtServUUID[100];
uint8_t advtServUUIDlen = 1;
uint16_t device_role = 0xFF;
discoveryContext_t discovery;

extern uint8_t  mtu_exchanged;
extern uint8_t  mtu_exchanged_wait;
extern uint16_t connection_handle;
extern uint16_t write_char_len;
extern volatile int app_flags;


uint8_t local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME,'S','T','M','-','K','E','Y','B','O','A','R','D'};

void APP_UserEvtRx(void *pData)
    {


    uint32_t i;
    hci_spi_pckt *hci_pckt =(hci_spi_pckt *)pData;
    //printf("PACKET TYPE: %d \r\n", hci_pckt->type);
   // hci_event_pckt* event_pckt = (hci_event_pckt*)hci_pckt->data;
    //printf("EVENT TYPE: %d \r\n", event_pckt->evt);
    if(hci_pckt->type==HCI_EVENT_PKT)
        {


        //Get data from packet
        hci_event_pckt *event_pckt =(hci_event_pckt*)hci_pckt->data;
        //process meta data
        if(event_pckt->evt ==EVT_LE_META_EVENT)
            {
            //get meta data
            evt_le_meta_event *evt =(void *)event_pckt->data;
            //process each meta data;
            for (i=0;i<(sizeof(hci_le_meta_events_table))/(sizeof(hci_le_meta_events_table_type));i++)
                {
                    if(evt->subevent ==hci_le_meta_events_table[i].evt_code)
                    {
                 /*       if (APP_FLAG(CONNECTED) {
                            printf("HCI_EVENT_PKT: EVT_LE_META_EVENT: %d \r\n", i);
                        }*/
                        hci_le_meta_events_table[i].process((void *)evt->data);
                    }
                }
            }
        //process vendor event
        else if(event_pckt->evt==EVT_VENDOR)
                {
                evt_blue_aci *blue_evt= (void *)event_pckt->data;
                for (i=0;i<(sizeof(hci_vendor_specific_events_table)/sizeof(hci_vendor_specific_events_table_type));i++)
                    {
                    if(blue_evt->ecode==hci_vendor_specific_events_table[i].evt_code)
                        {
                  /*      if (APP_FLAG(CONNECTED) {
                            printf("HCI_EVENT_PKT: EVT_VENDOR: %d \r\n", i);
                        }
                        */
                        hci_vendor_specific_events_table[i].process((void*)blue_evt->data);

                        }

                    }
                }
        else
            {
            for (i=0; i<(sizeof(hci_events_table)/sizeof(hci_events_table_type));i++)
                    {

                    if(event_pckt->evt==hci_events_table[i].evt_code)
                        {
                    /*
                     *  if (APP_FLAG(CONNECTED) {
                            printf("OTHER: OTHER: %d \r\n", i);
                        }

                        */
                        hci_events_table[i].process((void*)event_pckt->data);
                        }

                    }
            }
        }

    }



/* ***************** BlueNRG-1 Stack Callbacks ********************************/

/*******************************************************************************
 * Function Name  : aci_gap_proc_complete_event.
 * Description    : This event indicates the end of a GAP procedure.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
 void aci_gap_proc_complete_event(uint8_t Procedure_Code,
                                  uint8_t Status,
                                  uint8_t Data_Length,
                                  uint8_t Data[])
 {
   if (Procedure_Code == GAP_GENERAL_DISCOVERY_PROC)
   {
     /* gap procedure complete has been raised as consequence of a GAP
        terminate procedure done after a device found event during the discovery procedure */
     if (discovery.do_connect == TRUE)
     {
       discovery.do_connect = FALSE;
       discovery.check_disc_proc_timer = FALSE;
       discovery.startTime = 0;
       /* discovery procedure has been completed and no device found:
          go to discovery mode */
       discovery.device_state = DO_DIRECT_CONNECTION_PROC;
     }
     else
     {
       /* discovery procedure has been completed and no device found:
          go to discovery mode */
       discovery.check_disc_proc_timer = FALSE;
       discovery.startTime = 0;
       discovery.device_state = ENTER_DISCOVERY_MODE;
     }
   }
 }

















/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates the end of a connection procedure.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)

{
  /* Set the exit state for the Connection state machine: APP_FLAG_CLEAR(SET_CONNECTABLE); */
  APP_FLAG_CLEAR(SET_CONNECTABLE);

  discovery.check_disc_proc_timer = FALSE;
  discovery.check_disc_mode_timer = FALSE;
  discovery.startTime = 0;

  connection_handle = Connection_Handle;

  APP_FLAG_SET(CONNECTED);
  printf("CONNECTED!!!___________xS \r\n");

  discovery.device_state = INIT_STATE;

  /* store device role */
  device_role = Role;

  PRINT_DBG("Connection Complete with peer address: ");
  for (uint8_t i = 5; i > 0; i--)
  {
    PRINT_DBG("%02X-", Peer_Address[i]);
  }
  PRINT_DBG("%02X\r\n", Peer_Address[0]);

} /* end hci_le_connection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event.
 * Description    : This event indicates the discconnection from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{
  APP_FLAG_CLEAR(CONNECTED);

  /* Make the device connectable again. */
  APP_FLAG_SET(SET_CONNECTABLE);

  APP_FLAG_CLEAR(NOTIFICATIONS_ENABLED);

  APP_FLAG_CLEAR(START_READ_TX_CHAR_HANDLE);
  APP_FLAG_CLEAR(END_READ_TX_CHAR_HANDLE);
  APP_FLAG_CLEAR(START_READ_RX_CHAR_HANDLE);
  APP_FLAG_CLEAR(END_READ_RX_CHAR_HANDLE);
  APP_FLAG_CLEAR(TX_BUFFER_FULL);

  printf("Disconnection with reason: 0x%02X\r\n", Reason);
  Reset_DiscoveryContext();

} /* end hci_disconnection_complete_event() */

void aci_gap_pairing_complete_event(uint16_t Connection_Handle,
                                    uint8_t Status,
                                    uint8_t Reason)
{
  BLUENRG_PRINTF("aci_gap_pairing_complete_event\r\n");
}

/*******************************************************************************
 * Function Name  : hci_le_advertising_report_event.
 * Description    : An advertising report is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_advertising_report_event(uint8_t Num_Reports,
                                     Advertising_Report_t Advertising_Report[])
{
  /* Advertising_Report contains all the expected parameters */
  uint8_t evt_type = Advertising_Report[0].Event_Type;
  uint8_t data_length = Advertising_Report[0].Length_Data;
  uint8_t bdaddr_type = Advertising_Report[0].Address_Type;
  uint8_t bdaddr[6];

  BLUENRG_memcpy(bdaddr, Advertising_Report[0].Address, 6);

  /* BLE SampleApp device not yet found: check current device found */
  if (!(discovery.is_device_found))
  {
    /* BLE SampleApp device not yet found: check current device found */
    if ((evt_type == ADV_IND) && Find_DeviceName(data_length, Advertising_Report[0].Data))
    {
      discovery.is_device_found = TRUE;
      discovery.do_connect = TRUE;
      discovery.check_disc_proc_timer = FALSE;
      discovery.check_disc_mode_timer = FALSE;
      /* store first device found:  address type and address value */
      discovery.device_found_address_type = bdaddr_type;
      BLUENRG_memcpy(discovery.device_found_address, bdaddr, 6);
      /* device is found: terminate discovery procedure */
      discovery.device_state = DO_TERMINATE_GAP_PROC;
      PRINT_DBG("Device found\r\n");
    }
  }
} /* hci_le_advertising_report_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : Attribute modified from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
  Attribute_Modified_CB(Attr_Handle, Attr_Data_Length, Attr_Data);
} /* end aci_gatt_attribute_modified_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_notification_event.
 * Description    : Notification received from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/

extern uint16_t input_report_char_handle;
extern uint16_t output_report_char_handle;
void aci_gatt_notification_event(uint16_t Connection_Handle,
                                 uint16_t Attribute_Handle,
                                 uint8_t Attribute_Value_Length,
                                 uint8_t Attribute_Value[])
{
  printf("____aci_gatt_notification_event with output_report_char_handle %d and Attribute_Handle %d \r\n", output_report_char_handle, Attribute_Handle);
  if (Attribute_Handle == output_report_char_handle + 1)
  {
    printf("____ENTERED TO RECEIVE DATA \n");
    receiveData(Attribute_Value, Attribute_Value_Length);
  }

} /* end aci_gatt_notification_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_disc_read_char_by_uuid_resp_event.
 * Description    : Read characteristic by UUID from a peer device.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/

void aci_gatt_disc_read_char_by_uuid_resp_event(uint16_t Connection_Handle,
                                                uint16_t Attribute_Handle,
                                                uint8_t Attribute_Value_Length,
                                                uint8_t Attribute_Value[])
{
  PRINT_DBG("aci_gatt_disc_read_char_by_uuid_resp_event, Connection Handle: 0x%04X\r\n", Connection_Handle);
  if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
  {
    input_report_char_handle = Attribute_Handle;
    PRINT_DBG("TX Char Handle 0x%04X\r\n", tx_handle);
  }
  else
  {
    if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
    {
      output_report_char_handle = Attribute_Handle;
      PRINT_DBG("RX Char Handle 0x%04X\r\n", rx_handle);
      /**
       * LED blinking on the CENTRAL device to indicate the characteristic
       * discovery process has terminated
       */
      for (uint8_t i = 0; i < 9; i++)
      {
        HAL_GPIO_TogglePin(GPIOA, LED_Pin);
        HAL_Delay(250);
      }
    }
  }

} /* end aci_gatt_disc_read_char_by_uuid_resp_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_proc_complete_event.
 * Description    : GATT procedure complete event.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_proc_complete_event(uint16_t Connection_Handle,
                                  uint8_t Error_Code)
{
  if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
  {
    APP_FLAG_SET(END_READ_TX_CHAR_HANDLE);
  }
  else
  {
    if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
    {
      APP_FLAG_SET(END_READ_RX_CHAR_HANDLE);
    }
  }
} /* end aci_gatt_proc_complete_event() */

/*******************************************************************************
 * Function Name  : aci_gatt_tx_pool_available_event.
 * Description    : GATT TX pool available event.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers)
{
  APP_FLAG_CLEAR(TX_BUFFER_FULL);
} /* end aci_gatt_tx_pool_available_event() */

/*******************************************************************************
 * Function Name  : aci_att_exchange_mtu_resp_event.
 * Description    : GATT ATT exchange MTU response event.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_att_exchange_mtu_resp_event(uint16_t Connection_Handle,
                                     uint16_t Server_RX_MTU)
{
  PRINT_DBG("aci_att_exchange_mtu_resp_event: Server_RX_MTU=%d\r\n", Server_RX_MTU);

  if (Server_RX_MTU <= CLIENT_MAX_MTU_SIZE)
  {
    write_char_len = Server_RX_MTU - 3;
  }
  else
  {
    write_char_len = CLIENT_MAX_MTU_SIZE - 3;
  }

  if ((mtu_exchanged_wait == 0) || ((mtu_exchanged_wait == 1)))
  {
    /**
     * The aci_att_exchange_mtu_resp_event is received also if the
     * aci_gatt_exchange_config is called by the other peer.
     * Here we manage this case.
     */
    if (mtu_exchanged_wait == 0)
    {
      mtu_exchanged_wait = 2;
    }
    mtu_exchanged = 1;
  }
}

void BLE_Profile_Add_Advertisment_Service_UUID(uint16_t servUUID)
{
  uint8_t indx = advtServUUIDlen;

  advtServUUID[indx] = (uint8_t)(servUUID & 0xFF);
  indx++;
  advtServUUID[indx] = (uint8_t)(servUUID >> 8) & 0xFF;
  indx++;
  advtServUUIDlen = indx;
}

/*******************************************************************************
* Function Name  : Attribute_Modified_CB
* Description    : Attribute modified callback.
* Input          : Attribute handle modified.
*                  Length of the data.
*                  Attribute data.
* Return         : None.
*******************************************************************************/

void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data)
{
  printf("Entered Attribute_Modified_CB with %d\r\n",handle);
  if(handle == output_report_char_handle + 1)
  {
    receiveData(att_data, data_length);
  }
  else if(handle == input_report_char_handle + 2)
  {
    if(att_data[0] == 0x01)
    {
      printf("NOTIFICATIONS_ENABLED\r\n");

      APP_FLAG_SET(NOTIFICATIONS_ENABLED);
    }
    else if(att_data[0] == 0x00)
    {
      printf("NOTIFICATIONS_DISABLED\r\n");

      APP_FLAG_CLEAR(NOTIFICATIONS_ENABLED);
    }

  }
}








/*******************************************************************************
* Function Name  : Find_DeviceName.
* Description    : Extracts the device name.
* Input          : Data length.
*                  Data value
* Return         : TRUE if the local name found is the expected one, FALSE otherwise.
*******************************************************************************/





uint8_t Find_DeviceName(uint8_t data_length, uint8_t *data_value)
{
  uint8_t index = 0;

  while (index < data_length)
  {
    /* Advertising data fields: len, type, values */
    /* Check if field is complete local name and the length is the expected one for BLE SampleApp  */
    if (data_value[index+1] == AD_TYPE_COMPLETE_LOCAL_NAME)
    {
      /* check if found device name is the expected one: local_name */
      if (BLUENRG_memcmp(&data_value[index+1], &local_name[0], BLE_SAMPLE_APP_COMPLETE_LOCAL_NAME_SIZE) == 0)
      {
        return TRUE;
      }
      else
      {
        return FALSE;
      }
    }
    else
    {
      /* move to next advertising field */
      index += (data_value[index] +1);
    }
  }

  return FALSE;
}


