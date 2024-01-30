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


uint8_t advtServUUID[100];
uint8_t advtServUUIDlen=1;
uint16_t discovery_time     = 3000;
uint8_t  device_role        = 0xFF;
uint8_t  mtu_exchanged      = 0;
uint8_t  mtu_exchanged_wait = 0;
uint16_t write_char_len     			= CHAR_VALUE_LENGTH-3;
uint8_t  data[CHAR_VALUE_LENGTH-3];
uint8_t  counter         				= 0;
uint8_t  protocolMode    				= 0;

uint8_t btn_state;
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

#define DEBUG 1

#define APP_TIMER 1
#define IDLE_CONNECTION_TIMEOUT (120*1000) // 2 min

#define KEY_TABLE_LEN 33

#define  ADV_INTERVAL_MIN_MS  30
#define  ADV_INTERVAL_MAX_MS  50
uint8_t  local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME,'S','T','M','-','K','E','Y','B','O','A','R','D'};
uint8_t  isPaired=0;

//extern uint8_t  set_connectable;
volatile int app_flags = SET_CONNECTABLE;

/* discovery procedure mode context */
typedef struct discoveryContext_s
{
    uint8_t check_disc_proc_timer;
    uint8_t check_disc_mode_timer;
    uint8_t is_device_found;
    uint8_t do_connect;
    uint32_t startTime;
    uint8_t device_found_address_type;
    uint8_t device_found_address[6];
    uint16_t device_state;
} discoveryContext_t;

static discoveryContext_t discovery;

struct{
 uint16_t min_int;
 uint16_t max_int;
 uint16_t slave_lat;
 uint16_t timeout;
}ppcp;


extern uint16_t hid_service_handle;
extern uint16_t protocol_mode_char_handle,
input_report_char_handle, output_report_char_handle, feature_report_char_handle,
report_map_char_handle, boot_keyboard_input_char_handle,
boot_keyboard_output_char_handle,hid_info_char_handle, hid_control_point_char_handle;


extern uint8_t notifications;
extern uint16_t connection_handle;

void bluenrg_init(void)
	{
	tBleStatus ret;
	/*
	 * 1.Initialize HCI
	 * 2.Reset HCI
	 * 3.Configure Device address
	 * 4.Initialize GATT server
	 * 5.Initialize GAP service
	 * 6.Update characteristics
	 * 7.Add custom service
	 * */
	/*1.*/

    
	hci_init(APP_UserEvtRx,NULL);
	hci_reset();
	HAL_Delay(100);



	//add custom service
	ret = Configure_HidPeripheral();
    Reset_DiscoveryContext();
	if(ret !=BLE_STATUS_SUCCESS){printf("add_simple_service failed \r\n");}
	}
  uint8_t protocolModeSet=0;

void bluenrg_process(void)
		{   
			//process user events
	uint8_t data[]={'A','b'};
	hci_user_evt_proc();
    
    if (APP_FLAG(SET_CONNECTABLE)) {
        
        Connection_StateMachine();

    }
    
      if (APP_FLAG(CONNECTED) && !protocolModeSet && isPaired)
      {
    	protocolMode=REPORT_PROTOCOL_MODE;
        printf("protocol Mode was set to 1\r\n");
        aci_gatt_update_char_value(hid_service_handle,protocol_mode_char_handle,0,1,&protocolMode);
        protocolModeSet=1;
      }
      if (APP_FLAG(CONNECTED) && isPaired)
      {
        if ((mtu_exchanged == 0) && (mtu_exchanged_wait == 0))
      {
        printf("ROLE SLAVE (mtu_exchanged %d, mtu_exchanged_wait %d)\r\n",
                  mtu_exchanged, mtu_exchanged_wait);

        mtu_exchanged_wait = 1;
        uint8_t ret = aci_gatt_exchange_config(connection_handle);
        if (ret != BLE_STATUS_SUCCESS) {
          printf("aci_gatt_exchange_configuration error 0x%02x\r\n", ret);
        }
      }
      }
      if (APP_FLAG(CONNECTED) && APP_FLAG(NOTIFICATIONS_ENABLED) && isPaired)
      {
    	  printf("Trying to print smth\r\n");
    	  processInputData(data,sizeof(data));
    	  HAL_Delay(1000);

      }
      //User_Process();

		}




uint8_t hidDevice_Init(uint8_t IO_Capability, connParam_Type connParam,
                       uint8_t dev_name_len, uint8_t *dev_name, uint8_t *addr)
{
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  uint8_t ret;
  //TODO: What to do with connParam?

  /* Set the device public address */
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, addr);
  if(ret != BLE_STATUS_SUCCESS)
  {
      printf("Setting BD_ADDR failed 0x%02x\r\n", ret);
  }
  else
  {
    printf("Public address: ");
    for (uint8_t i=5; i>0; i--)
    {
      printf("%02X-", addr[i]);
    }
    printf("%02X\r\n", addr[0]);
  }

   /* GATT Init */
  ret = aci_gatt_init();
  if (ret != BLE_STATUS_SUCCESS) {
    printf("aci_gatt_init() failed: 0x%02x\r\n", ret);
    return ret;
  }
  else
    printf("aci_gatt_init() --> SUCCESS\r\n");

  /* GAP Init */
  uint8_t appareance[]={0xC1,0x03};

  ret = aci_gap_init(GAP_CENTRAL_ROLE | GAP_PERIPHERAL_ROLE,0x0,0x20, &service_handle,
                     &dev_name_char_handle, &appearance_char_handle);
  aci_gatt_update_char_value(service_handle, appearance_char_handle, 0, 2, appareance);

  ret = aci_gatt_update_char_value_ext(0, service_handle, dev_name_char_handle, 0,dev_name_len, 0, dev_name_len, dev_name);
  ret = aci_gatt_update_char_value_ext(0, service_handle, appearance_char_handle+2, 0,sizeof(connParam), 0, sizeof(connParam), (uint8_t*)&connParam);

  /* Set IO capability */
  ret=aci_gap_set_io_capability(IO_Capability);
  if(ret!=BLE_STATUS_SUCCESS)
  {
    printf("GAP_set_io failed: 0x%02x\r\n", ret);
  }
  return ret;
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
    printf("aci_gap_set_authentication_requirement()failed: 0x%02x\r\n", ret);
  }
  else
    printf("aci_gap_set_authentication_requirement() --> SUCCESS\r\n");
  return ret;
}

void hidSetIdleTimeout(uint32_t timeout)
{
  //TODO: Implement me
  return;
}

uint8_t hidSetTxPower(uint8_t level)
{
  uint8_t ret;
  ret=aci_hal_set_tx_power_level(1, level);
  if (ret != BLE_STATUS_SUCCESS) {
    printf("Error in aci_hal_set_tx_power_level() 0x%04x\r\n", ret);
  }
  return ret;
}

uint8_t hidSetDeviceDiscoverable(uint8_t mode, uint8_t nameLen, uint8_t *name)
{
  uint8_t ret;
  advtServUUID[0]=AD_TYPE_16_BIT_SERV_UUID;

        // advtServUUID[1]=HUMAN_INTERFACE_DEVICE_SERVICE_UUID;
        // advtServUUID[2]=BATTERY_SERVICE_SERVICE_UUID;
        // advtServUUID[3]=DEVICE_INFORMATION_SERVICE_UUID;
        // advtServUUID[100]=0x1111;

   BLE_Profile_Add_Advertisment_Service_UUID(HUMAN_INTERFACE_DEVICE_SERVICE_UUID);
   BLE_Profile_Add_Advertisment_Service_UUID(BATTERY_SERVICE_SERVICE_UUID);
   BLE_Profile_Add_Advertisment_Service_UUID(DEVICE_INFORMATION_SERVICE_UUID);
  if (mode==LIMITED_DISCOVERABLE_MODE)
  {
    printf("Set limited Discoverable Mode.\n");
    ret = aci_gap_set_limited_discoverable(ADV_IND,
                                  (ADV_INTERVAL_MIN_MS*1000)/625,(ADV_INTERVAL_MAX_MS*1000)/625,
								  PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                  nameLen, name, 0, NULL, 0, 0);
    if(ret != BLE_STATUS_SUCCESS)
    {
      printf("aci_gap_set_discoverable() failed: 0x%02x\r\n",ret);
    }
    else
    {
    	printf("aci_gap_set_discoverable() LIMITED --> SUCCESS\r\n");
    }
    return ret;
  }
  else if(mode==GENERAL_DISCOVERABLE_MODE)
  {
    //printf("Set General Discoverable Mode.\n");
    ret = aci_gap_set_discoverable(ADV_IND,
                                  (ADV_INTERVAL_MIN_MS*1000)/625,(ADV_INTERVAL_MAX_MS*1000)/625,
                                  STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                  nameLen, name, 0, NULL, 0, 0);
    if(ret != BLE_STATUS_SUCCESS)
    {
      //printf("aci_gap_set_discoverable() failed: 0x%02x\r\n",ret);
    }
    else
    {
    	printf("aci_gap_set_discoverable() GENERAL--> SUCCESS\r\n");
    }

    return ret;
  }
  printf("Wrong mode, device couldn't set as discoverable.\n");
  return -1;

}
static void receiveData(uint8_t* data_buffer, uint8_t Nb_bytes)
{
  printf("Received: ");
  for(int i = 0; i < Nb_bytes; i++)
  {
    printf("%c", data_buffer[i]);
  }
  printf("\r\n");
  fflush(stdout);
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
   for (uint8_t i=5; i>0; i--)
   {
     PRINT_DBG("%02X-", Peer_Address[i]);
   }
   PRINT_DBG("%02X\r\n", Peer_Address[0]);

 }/* end hci_le_connection_complete_event() */

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

 }/* end hci_disconnection_complete_event() */



 void aci_gap_pairing_complete_event(uint16_t Connection_Handle,
                                     uint8_t Status,
                                     uint8_t Reason)
 {
   BLUENRG_PRINTF("aci_gap_pairing_complete_event\r\n");
   isPaired=1;


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
   uint8_t evt_type = Advertising_Report[0].Event_Type ;
   uint8_t data_length = Advertising_Report[0].Length_Data;
   uint8_t bdaddr_type = Advertising_Report[0].Address_Type;
   uint8_t bdaddr[6];

   BLUENRG_memcpy(bdaddr, Advertising_Report[0].Address,6);

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
  printf("____aci_gatt_notification_event with output_report_char_handle %d and Attribute_Handle %d \r\n",output_report_char_handle,Attribute_Handle);
  if(Attribute_Handle == output_report_char_handle+1)
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
     if(APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
     {
    	 output_report_char_handle = Attribute_Handle;
       PRINT_DBG("RX Char Handle 0x%04X\r\n", rx_handle);
       /**
        * LED blinking on the CENTRAL device to indicate the characteristic
        * discovery process has terminated
        */
       for (uint8_t i = 0; i < 9; i++) {
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

  if (Server_RX_MTU <= CLIENT_MAX_MTU_SIZE) {
    write_char_len = Server_RX_MTU - 3;
  }
  else {
    write_char_len = CLIENT_MAX_MTU_SIZE - 3;
  }




  if ((mtu_exchanged_wait == 0) || ((mtu_exchanged_wait == 1))) {
    /**
     * The aci_att_exchange_mtu_resp_event is received also if the
     * aci_gatt_exchange_config is called by the other peer.
     * Here we manage this case.
     */
    if (mtu_exchanged_wait == 0) {
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

    for (uint16_t i = 0; i < (CHAR_VALUE_LENGTH - 3); i++) {
        data[i] = 0x31 + (i % 10);
        if ((i + 1) % 10 == 0) {
            data[i] = 'x';
        }
    }
}


void Connection_StateMachine(void)
{
    uint8_t ret;

    switch (discovery.device_state)
    {
    case (INIT_STATE):
    {
        printf("INSIDE INIT STATE\r\n");
        Reset_DiscoveryContext();
        discovery.device_state = START_DISCOVERY_PROC;
    }
    break; /* end case (INIT_STATE) */
    case (START_DISCOVERY_PROC):
    {
      //  printf("INSIDE START_DISCOVERY_PROC\r\n");

        /*
        ADV_IND,
        (ADV_INTERVAL_MIN_MS*1000)/625,
        (ADV_INTERVAL_MAX_MS*1000)/625,
        */


        ret = aci_gap_start_general_discovery_proc((ADV_INTERVAL_MIN_MS * 1000) / 625,
            (ADV_INTERVAL_MAX_MS * 1000) / 625, 
            PUBLIC_ADDR,
            0x00);
        //ret = aci_gap_start_limited_discovery_proc(SCAN_P, SCAN_L, PUBLIC_ADDR, 0x00);
        if (ret != BLE_STATUS_SUCCESS)
        {
            printf("aci_gap_start_limited_discovery_proc() failed: %02X\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            printf("aci_gap_start_limited_discovery_proc OK\r\n");
            discovery.startTime = HAL_GetTick(); /**************************TODO**********/
            discovery.check_disc_proc_timer = TRUE;
            discovery.check_disc_mode_timer = FALSE;
            discovery.device_state = WAIT_TIMER_EXPIRED;
        }
    }
    break;/* end case (START_DISCOVERY_PROC) */
    case (WAIT_TIMER_EXPIRED):
    {
     //   printf("INSIDE WAIT_TIMER_EXPIRED\r\n");
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




    /*********************************************/




    case (DO_NON_DISCOVERABLE_MODE):
    {
       // printf("INSIDE DO_NON_DISCOVERABLE_MODE\r\n");
        ret = aci_gap_set_non_discoverable();
        if (ret != BLE_STATUS_SUCCESS)
        {
            printf("aci_gap_set_non_discoverable() failed: 0x%02x\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            printf("aci_gap_set_non_discoverable() OK\r\n");
            /* Restart Central discovery procedure */
            discovery.device_state = INIT_STATE;
        }
    }
    break; /* end case (DO_NON_DISCOVERABLE_MODE) */
    case (DO_TERMINATE_GAP_PROC):
    {
       // printf("INSIDE DO_TERMINATE_GAP_PROC\r\n");
        /* terminate gap procedure */
        ret = aci_gap_terminate_gap_proc(0x02); // GENERAL_DISCOVERY_PROCEDURE
        if (ret != BLE_STATUS_SUCCESS)
        {
            printf("aci_gap_terminate_gap_procedure() failed: 0x%02x\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
            break;
        }
        else
        {
            printf("aci_gap_terminate_gap_procedure() OK\r\n");
            discovery.device_state = WAIT_EVENT; /* wait for GAP procedure complete */
        }
    }
    break; /* end case (DO_TERMINATE_GAP_PROC) */
    case (DO_DIRECT_CONNECTION_PROC):
    {
      //  printf("INSIDE DO_DIRECT_CONNECTION_PROC\r\n");
        printf("Device Found with address: ");
        for (uint8_t i = 5; i > 0; i--)
        {
            printf("%02X-", discovery.device_found_address[i]);
        }
        printf("%02X\r\n", discovery.device_found_address[0]);
        /* Do connection with first discovered device */
        ret = aci_gap_create_connection(SCAN_P, SCAN_L,
            discovery.device_found_address_type, discovery.device_found_address,
            PUBLIC_ADDR, 40, 40, 0, 60, 2000, 2000);
        if (ret != BLE_STATUS_SUCCESS)
        {
            printf("aci_gap_create_connection() failed: 0x%02x\r\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            printf("aci_gap_create_connection() OK\r\n");
            discovery.device_state = WAIT_EVENT;
        }
    }
    break; /* end case (DO_DIRECT_CONNECTION_PROC) */
    case (WAIT_EVENT):
    {
    //    printf("INSIDE WAIT_EVENT\r\n");
        discovery.device_state = WAIT_EVENT;
    }
    break; /* end case (WAIT_EVENT) */
    
    
    
    //******//


    case (ENTER_DISCOVERY_MODE):
    {
    //    printf("INSIDE ENTER_DISCOVERY_MODE\r\n");
        /* Put Peripheral device in discoverable mode */

        /* disable scan response */
        hci_le_set_scan_response_data(0, NULL);

        /*CHANGEEEDDDDDDDDDDDDDDDDDDDDDDDDDDD**/
        ret = hidSetDeviceDiscoverable(LIMITED_DISCOVERABLE_MODE, sizeof(local_name), local_name);
        /*ret = aci_gap_set_discoverable(ADV_DATA_TYPE, ADV_INTERV_MIN, ADV_INTERV_MAX, PUBLIC_ADDR,
            NO_WHITE_LIST_USE, sizeof(local_name), local_name, 0, NULL,
            0x0, 0x0);*/


        if (ret != BLE_STATUS_SUCCESS) {
            printf("Error in hidSetDeviceDiscoverable() 0x%02x\n", ret);
            discovery.device_state = DISCOVERY_ERROR;
        }
        else
        {
            printf("aci_gap_set_discoverable() OK\r\n");
            discovery.startTime = HAL_GetTick();
            discovery.check_disc_mode_timer = TRUE;
            discovery.check_disc_proc_timer = FALSE;
            discovery.device_state = WAIT_TIMER_EXPIRED;
        }
    }
    break; /* end case (ENTER_DISCOVERY_MODE) */
    case (DISCOVERY_ERROR):
    {
     //   printf("INSIDE DISCOVERY_ERROR\r\n");
        Reset_DiscoveryContext();
    }
    break; /* end case (DISCOVERY_ERROR) */
    default:
        break;
    }/* end switch */

}/* end Connection_StateMachine() */
