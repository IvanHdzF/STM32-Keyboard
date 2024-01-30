/*
 * services.h
 *
 *  Created on: Dec 9, 2021
 *      Author: hussamaldean
 */

#ifndef INC_SERVICES_H_
#define INC_SERVICES_H_
#include "app_bluenrg.h"
#include "bluenrg_conf.h"
#include "bluenrg1_types.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_hci_le.h"
#include "stdio.h"
#include "stdlib.h"








/*Update types for update char value ext*/
#define GATT_LOCAL_UPDATE (0x00)
#define GATT_NOTIFICATION (0x01)
#define GATT_INDICATION (0x02)
#define GATT_DISABLE_RETRANSMISSIONS  (0x04)





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
#define MANUFAC_NAME_LEN 9
#define MODEL_NUMB_LEN 4
#define FW_REV_LEN 4
#define SW_REV_LEN 4
#define PNP_ID_LEN 7
#define REPORT_NUMBER 2
#define EXTERNAL_REPORT_NUMBER 1
#define INCLUDED_SERVICE_NUMBER 1
#define REPORT_ID 0x01
#define INPUT_REPORT 0x01
#define OUTPUT_REPORT 0x02

#define CLIENT_CHAR_DESCR_UUID (0x2902)

/* UUIDs for battery service */
#define BATTERY_SERVICE_SERVICE_UUID                        (0x180F)
#define BATTERY_LEVEL_CHAR_UUID                             (0x2A19)
#define CHAR_PRESENTATION_FORMAT_DESCRIPTOR_UUID            (0x2904)


/*
 * UUIDs for Device Information Service
 */
#define DEVICE_INFORMATION_SERVICE_UUID (0x180A)
#define SYSTEM_ID_UUID                  (0x2A23)
#define MODEL_NUMBER_UUID               (0x2A24)
#define SERIAL_NUMBER_UUID              (0x2A25)
#define FIRMWARE_REVISION_UUID          (0x2A26)
#define HARDWARE_REVISION_UUID          (0x2A27)
#define SOFTWARE_REVISION_UUID          (0x2A28)
#define MANUFACTURER_NAME_UUID          (0x2A29)
#define IEEE_CERTIFICATION_UUID         (0x2A2A)
#define PNP_ID_UUID                     (0x2A50)


/*
 *  UUIDs for HID Service
 *  */
#define HUMAN_INTERFACE_DEVICE_SERVICE_UUID                 (0x1812)
#define PROTOCOL_MODE_CHAR_UUID                             (0x2A4E)
#define REPORT_CHAR_UUID                                    (0x2A4D)
#define REPORT_MAP_CHAR_UUID                                (0x2A4B)
#define BOOT_KEYBOARD_INPUT_REPORT_CHAR_UUID                (0x2A22)
#define BOOT_KEYBOARD_OUTPUT_REPORT_CHAR_UUID               (0x2A32)
#define BOOT_MOUSE_INPUT_REPORT_CHAR_UUID                   (0x2A33)
#define HID_INFORMATION_CHAR_UUID                           (0x2A4A)
#define HID_CONTROL_POINT_CHAR_UUID                         (0x2A4C)
#define HID_CLIENT_CHARACTERISTIC_CON_DESCRIPTOR            (0x2902)
#define HID_REPORT_REFERENCE_CHAR_DESCRIPTOR                (0x2908)
#define PERIPHERAL_PUBLIC_ADDRESS                            {0xBC, 0xFC, 0x00, 0xE1, 0x80, 0x02}

#define HID_EXTERNAL_REPORT_DESCRIPTOR 						(0x2907)
/**
 * @name HID/HOGP Device discoverable mode configuration constants
 *@{
 */

/**
 * @brief Limited discovery procedure constant
 */
#define LIMITED_DISCOVERABLE_MODE 0x01

/**
 * @brief General discovery procedure constant
 */
#define GENERAL_DISCOVERABLE_MODE 0x02

//@} \\END HID/HOGP Device discoverable mode configuration constants



/**
 * This sample application uses a char value length greater than 20 bytes
 * (typically used).
 * For using greater values for CHAR_VALUE_LENGTH (max 512) and
 * CLIENT_MAX_MTU_SIZE (max 247):
 * - increase both the two #define below to their max values
 * - increase both the HCI_READ_PACKET_SIZE and HCI_MAX_PAYLOAD_SIZE to 256
 *   (file bluenrg_conf.h)
 * - increase the CSTACK in the IDE project options (0xC00 is enough)
*/
#define CHAR_VALUE_LENGTH 63
#define CLIENT_MAX_MTU_SIZE 158



/**
 * @brief Included Service type definition
 */
typedef struct includeSerS {
  /** Service UUID to be included */
  uint16_t uuid;
  /** Start handle of the service to be included.
   *  If the service to include is BATTERY Service
   *  This data is not required
   */
  uint16_t start_handle;
  /** End handle of the service to be included.
   *  If the service to include is BATTERY Service
   *  This data is not required
   */
  uint16_t end_handle;
} includeSer_Type;


/**
 * @brief Report Type definition
 */
typedef struct reportS {
  /** Report ID */
  uint8_t ID;
  /** Report Type: 0x01 Input, 0x02 Output, 0x03 Feature */
  uint8_t type;
  /** Report Length */
  uint8_t length;
} report_Type;



/**
 * @brief Battery Service Specification
 */
typedef struct batteryServiceS {
  /** TRUE if the battery level is included in the report map.
   *  False otherwise
   */
  uint8_t inReportMap;
  /** Report ID if the battery level is included
   *  in the report map
   */
  uint8_t reportID;
} batteryService_Type;


/**
 * @brief Device Information Service Specification
 */
typedef struct devInfServiceS {
  /** Manufacter Name */
  uint8_t manufacName[MANUFAC_NAME_LEN];
  /** Model Number */
  uint8_t modelNumber[MODEL_NUMB_LEN];
  /** FW Revision */
  uint8_t fwRevision[FW_REV_LEN];
  /** SW Revision */
  uint8_t swRevision[SW_REV_LEN];
  /** PNP ID */
  uint8_t pnpID[PNP_ID_LEN];
} devInfService_Type;



/**
 * @brief HID Service Specification
 */
typedef struct hidServiceS {
  /** TRUE if the device supports the
   *  boot protocol mode. FALSE otherwise
   */
  uint8_t bootSupport;
  /** Report Descriptor length */
  uint8_t reportDescLen;
  /** Report Descriptor */
  uint8_t *reportDesc;
  /** TRUE if the device supports the
   *  Report characterisitic. FALSE otherwise
   */
  uint8_t reportSupport;
  /** Report Reference Descriptor */
  report_Type reportReferenceDesc[REPORT_NUMBER];
  /** TRUE if the HID device is a keyboard that
   *  supports boot protocol mode.
   *  FALSE otherwise.
   */
  uint8_t isBootDevKeyboard;
  /** TRUE if the HID device is a mouse that supports
   *  boot protocol mode.
   *  FALSE otherwise.
   */
  uint8_t isBootDevMouse;
  /** TRUE if the HID Service has external report.
   *  FALSE otherwise.
   */
  uint8_t externalReportEnabled;
  /** External reports UUID
   *  referenced in report Map
   */
  uint16_t externalReport[EXTERNAL_REPORT_NUMBER];
  /** TRUE if the HID service has included services.
   *  FALSE otherwise.
   */
  uint8_t includedServiceEnabled;
  /** Included services UUID */
  includeSer_Type includedService[INCLUDED_SERVICE_NUMBER];
  /** HID Information Characteristic value The format is:
   * - 2 bytes USB HID specification implemented by HID Device
   * - 1 byte Country Code
   * - 1 byte Bit0: RemoteWake, Bit1: Normally Connectable
   */
  uint8_t informationCharac[4];
} hidService_Type;

/**
 * @brief Connection parameter to request after the bonding and service discovery.
 * The HID Device requests to change to its preferred connection parameters which best
 * suit its use case
 */
typedef struct connParamS {
  /** Minimum value for the connection event interval.
   *  connIntervalMin = Interval Min * 1.25ms
   */
  uint16_t interval_min;
  /** Maximum value for the connection event interval.
   * connIntervalMax = Interval Max * 1.25ms
   */
  uint16_t interval_max;
  /** Slave latency parameter */
  uint16_t slave_latency;
  /** Connection timeout parameter.
   *  The value is Timeout Multiplier * 10ms
   */
  uint16_t timeout_multiplier;
} connParam_Type;



void APP_UserEvtRx(void *pData);
void send_data(uint8_t *data_buffer, uint8_t no_byte);
tBleStatus addBatteryService(void);
tBleStatus addDeviceInformationService(devInfService_Type* devInf);
void updateDIService(devInfService_Type* devInf);
tBleStatus addHumanInterfaceService(hidService_Type* hid);
void updateHIDServiceParams();
void BLE_Profile_Add_Advertisment_Service_UUID(uint16_t servUUID);
//static void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data);

uint8_t Configure_HidPeripheral(void);

/**
 * @name HOGP Device Configuration Functions
 *@{
 */

/**
  * @brief  Initializes the  HID Device with Peripheral Role and
  * sets the public address.
  * @param IO_Capability Sets the device IO capabilities. Possible value are:
  * - IO_CAP_DISPLAY_ONLY
  * - IO_CAP_DISPLAY_YES_NO
  * - KEYBOARD_ONLY
  * - IO_CAP_NO_INPUT_NO_OUTPUT
  * - IO_CAP_KEYBOARD_DISPLAY
  * @param connParam Connection parameter to send to the
  * HID Host after bonding/service discovery
  * @param dev_name_len Device name length
  * @param dev_name Device name
  * @param addr Public device address
  * @retval Status of the call.
  */
uint8_t hidDevice_Init(uint8_t IO_Capability, connParam_Type connParam,
                       uint8_t dev_name_len, uint8_t *dev_name, uint8_t *addr);

/**
 * @brief Sets the Idle Connection Timeout according the HID over GATT profile specification.
 * The HID device may perform the GAP Terminate Connection procedure if the connection is
 * idle for a time period, which is implementation dependent.
 * After bonding the HID device should wait for this idle connection timeout to allow the HID
 * Host to complete the configuration procedure.
 * Default value 500 ms.
 * @note If the timeout is zero the Power Save management is disabled
 * @param timeout Idle connection timeout expressed in number of millisecond.
 * @retval Nothing
 */
void hidSetIdleTimeout(uint32_t timeout);

//@} \\END HOGP Device Configuration Functions

/**
 * @name HOGP Security Functions
 *@{
 */

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

/**
 * @name HOGP GATT Database Configuration Functions
 *@{
 */

/**
 * @brief Adds the Primary service and the characteristics associated
 * for the battery, device information and hid services.
 * @param battery Battery Service characteristics to add.
 * See the tpyedef batteryService_Type for more details.
 * @param devInf Device Information Service characteristics to add.
 * See the typedef devInfService_Type for more details.
 * @param hid HID Service characteristics to add.
 * See the typedef HID Service for more details.
 * @retval Status of the call
 */
uint8_t hidAddServices(batteryService_Type* battery, devInfService_Type* devInf, hidService_Type* hid);

//@} \\END HOGP GATT Database Configuration Functions

/**
 * @name HOGP Discoverable Mode Functions
 *@{
 */

/**
  * @brief Starts the peripheral device discoverable mode. This mode is ended
  * when either the upper layer issues a command to terminate the procedure
  * using the command hidTerminateDiscoverableMode() or the
  * timeout happens (timeout value is 180 s).
  * @param mode Discoverable mode to use:
  * - LIMITED_DISCOVERABLE_MODE
  * - GENERAL_DISCOVERABLE_MODE
  * @param nameLen Local name length. Max 20
  * @param name Local name value
  * @retval Status of the call.
  */
uint8_t hidSetDeviceDiscoverable(uint8_t mode, uint8_t nameLen, uint8_t *name);

/**
 * @brief Sets the TX power level used during the communication
 * Default value is -2.1 dBm
 * @param level Power Level, valid range is [0 - 7]. The following table determines
 * the output power level (dBm):
 * - 0 = -14
 * - 1 = -11
 * - 2 = -8
 * - 3 = -5
 * - 4 = -2
 * - 5 = 2
 * - 6 = 4
 * - 7 = 8
 * @retval Status of the call
 */
uint8_t hidSetTxPower(uint8_t level);


/** Documentation for C struct Advertising_Report_t */
typedef struct Advertising_Report_t_s {
	/** Type of advertising report event:
  ADV_IND: Connectable undirected advertising',
  ADV_DIRECT_IND: Connectable directed advertising,
  ADV_SCAN_IND: Scannable undirected advertising,
  ADV_NONCONN_IND: Non connectable undirected advertising,
  SCAN_RSP: Scan response.
	* Values:
	- 0x00: ADV_IND
	- 0x01: ADV_DIRECT_IND
	- 0x02: ADV_SCAN_IND
	- 0x03: ADV_NONCONN_IND
	- 0x04: SCAN_RSP
	*/
	uint8_t Event_Type;
	/** 0x00 Public Device Address
  0x01 Random Device Address
  0x02 Public Identity Address (Corresponds to Resolved Private Address)
  0x03 Random (Static) Identity Address (Corresponds to Resolved Private Address)
	* Values:
	- 0x00: Public Device Address
	- 0x01: Random Device Address
	- 0x02: Public Identity Address
	- 0x03: Random (Static) Identity Address
	*/
	uint8_t Address_Type;
	/** Public Device Address, Random Device Address, Public Identity
  Address or Random (static) Identity Address of the advertising
  device.
	*/
	uint8_t Address[6];
	/** Length of the Data[i] field for each device which responded.
	* Values:
	- 0 ... 31
	*/
	uint8_t Length_Data;
	/** Length_Data[i] octets of advertising or scan response data formatted
  as defined in [Vol 3] Part C, Section 8.
	*/
	uint8_t* Data;
	/** N Size: 1 Octet (signed integer)
  Units: dBm
	* Values:
	- 127: RSSI not available
	- -127 ... 20
	*/
	uint8_t RSSI;
} Advertising_Report_t;

uint8_t Find_DeviceName(uint8_t data_length, uint8_t* data_value);



#endif /* INC_SERVICES_H_ */
