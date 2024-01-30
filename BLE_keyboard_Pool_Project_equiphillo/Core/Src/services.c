#include "app_bluenrg.h"
#include "services.h"
#include "bluenrg_conf.h"
#include "bluenrg1_types.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_hci_le.h"
#include "stdio.h"
#include "stdlib.h"
#include "main.h"


#define PERIPHERAL_PUBLIC_ADDRESS {0xBC, 0xFC, 0x00, 0xE1, 0x80, 0x02}


//uint8_t  connected=0;
//uint8_t  set_connectable=1;
uint16_t connection_handle=0;
uint8_t  notification_enabled=0;
uint8_t serviceMaxAttributeRecords, encrypKeySize;
uint16_t charValueLength;

extern volatile int app_flags;


// Keyboard report descriptor
uint8_t reportDesc[] = {
	    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	    0x09, 0x06, // USAGE (Keyboard)
	    0xA1, 0x01, // COLLECTION (Application)
	    0x05, 0x07, // USAGE_PAGE (Keyboard)
	    //0x85, 0x01,   // REPORT_ID (1)
	    // 1 byte Modifier: Ctrl, Shift and other modifier keys, 8 in total
	    0x19, 0xE0, // USAGE_MINIMUM (kbd LeftControl)
	    0x29, 0xE7, // USAGE_MAXIMUM (kbd Right GUI)
	    0x15, 0x00, // LOGICAL_MINIMUM (0)
	    0x25, 0x01, // LOGICAL_MAXIMUM (1)
	    0x75, 0x01, // REPORT_SIZE (1)
	    0x95, 0x08, // REPORT_COUNT (8)
	    0x81, 0x02, // INPUT (Data,Var,Abs)
	    // 1 Reserved byte
	    0x95, 0x01,  // REPORT_COUNT (1)
	    0x75, 0x08, // REPORT_SIZE (8)
	    0x81, 0x01, // INPUT (Cnst,Ary,Abs)
	    // LEDs for num lock etc
	    0x95, 0x05, // REPORT_COUNT (5)
	    0x75, 0x01, // REPORT_SIZE (1)
	    0x05, 0x08, // USAGE_PAGE (LEDs)
	    // 0x85, 0x01, // REPORT_ID (1)
	    0x19, 0x01, // USAGE_MINIMUM (Num Lock)
	    0x29, 0x05, // USAGE_MAXIMUM (Kana)
	    0x91, 0x02, // OUTPUT (Data,Var,Abs)
	    // Reserved 3 bits
	    0x95, 0x01, // REPORT_COUNT (1)
	    0x75, 0x03, // REPORT_SIZE (3)
	    0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
	    /* Slots for 6 keys that can be pressed down at the same time */
	    0x95, 0x06, // REPORT_COUNT (6)
	    0x75, 0x08, // REPORT_SIZE (8)
	    0x15, 0x00, // LOGICAL_MINIMUM (0)
	    0x25, 0x65, // LOGICAL_MAXIMUM (101)
	    0x05, 0x07, // USAGE_PAGE (Keyboard)
	    0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
	    0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
	    0x81, 0x00, // INPUT (Data,Ary,Abs)
	    0xC0, // END_COLLECTION
};



uint8_t dev_name[]={'S', 'T', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd'};
hidService_Type hid_param;
uint8_t ret;

uint8_t Configure_HidPeripheral(void)
{
	 uint8_t ret;
	  batteryService_Type battery;
	  devInfService_Type devInf;
	  hidService_Type hid;
	  connParam_Type connParam;
	  uint8_t addr[] = PERIPHERAL_PUBLIC_ADDRESS;
	  //uint8_t local_name[]={AD_TYPE_COMPLETE_LOCAL_NAME,'S', 'T', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd'};
	  /* HID Peripheral Init */
	  connParam.interval_min = 0x10;
	  connParam.interval_max = 0x14;
	  connParam.slave_latency = 0x14;
	  connParam.timeout_multiplier = 0xD2;
	  ret = hidDevice_Init(IO_CAP_DISPLAY_ONLY, connParam, sizeof(dev_name), dev_name, addr);
	  if (ret != BLE_STATUS_SUCCESS) {
	    printf("Error in hidDevice_Init() 0x%02x\n", ret);
	    return ret;
	  }

	  /* Set the HID Peripheral Security */
	  ret = hidSetDeviceSecurty(TRUE, USE_FIXED_PIN_FOR_PAIRING, 123456); //TODO: Maybe check this if app doesn't work
	  if (ret != BLE_STATUS_SUCCESS) {
	    printf("Error in hidSetDeviceSecurty() 0x%02x\n", ret);
	    return ret;
	  }

	  /* Set the HID Idle Timeout */
	  hidSetIdleTimeout(IDLE_CONNECTION_TIMEOUT);


	  /* Set the TX power -2 dBm */
	  ret = hidSetTxPower(4);

	  if (ret != BLE_STATUS_SUCCESS) {
	    printf("Error in hidSetTxPower() 0x%02x\n", ret);
	    return ret;
	  }

	  /**** Setup the GATT Database ****/

	  /* Battery Service */
	  battery.inReportMap = FALSE;

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

	  /* HID Service */
	  hid = hid_param;
	  hid.reportDescLen = sizeof(reportDesc);
	  hid.reportDesc = reportDesc;

	  ret = hidAddServices(&battery, &devInf, &hid);
	  if (ret != BLE_STATUS_SUCCESS) {
	    printf("Error in hidAddServices() 0x%02x\n", ret);
	    return ret;
	  }

	  /* Set the HID Peripheral device discoverable */
	/*  ret = hidSetDeviceDiscoverable(LIMITED_DISCOVERABLE_MODE, sizeof(local_name), local_name);
	  if (ret != BLE_STATUS_SUCCESS) {
	    printf("Error in hidSetDeviceDiscoverable() 0x%02x\n", ret);
	    return ret;
	  }
      */

	  printf("HID Keyboard Configured\r\n");



	  /* Button Init */



	  return BLE_STATUS_SUCCESS;
}


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
uint8_t hidAddServices(batteryService_Type* battery, devInfService_Type* devInf, hidService_Type* hid){

    ret = addBatteryService();
    ret = addDeviceInformationService(devInf);
    ret = addHumanInterfaceService(hid);



    return BLE_STATUS_SUCCESS;
}


uint16_t battery_service_handle;
uint16_t battery_level_char_handle;
uint16_t BATTERY_client_char_config_desc;

/**
 * @brief Adds BATTERY SERVICE (UUID 0x180F) with:
 * 1 characteristic (BATTERY LEVEL UUID 0x2A19)
 * Updates the value for BATTERY LEVEL to 50
 */
tBleStatus addBatteryService(){

    Service_UUID_t battery_service_uuid;
    Char_UUID_t battery_level_char_uuid;
    //Char_Desc_Uuid_t char_presentation_format_descriptor;

    battery_service_uuid.Service_UUID_16=BATTERY_SERVICE_SERVICE_UUID;
    battery_level_char_uuid.Char_UUID_16=BATTERY_LEVEL_CHAR_UUID ;


    serviceMaxAttributeRecords = 4;


    ret = aci_gatt_add_service(UUID_TYPE_16,
            &battery_service_uuid,
            PRIMARY_SERVICE,
            serviceMaxAttributeRecords,
            &battery_service_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
           printf("Error in Adding BATTERY SERVICE 0x%02x\n", ret);
           return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(battery_service_handle,
            UUID_TYPE_16,
            &battery_level_char_uuid,
            charValueLength,
            CHAR_PROP_READ,
            ATTR_PERMISSION_NONE,
            GATT_NOTIFY_ATTRIBUTE_WRITE,
            encrypKeySize,
            CHAR_VALUE_LEN_CONSTANT,
            &battery_level_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char for BATTERY SERVICE 0x%02x\n", ret);
        return ret;
    }
    uint8_t batteryLevel=75;
    ret = aci_gatt_update_char_value_ext(0,
            battery_service_handle,
            battery_level_char_handle,
            0,
            sizeof(batteryLevel),
            0,
            sizeof(batteryLevel),
            (uint8_t *)&batteryLevel);
    if(ret != BLE_STATUS_SUCCESS)
      {
        printf("Updating BATERRY level failed failed: 0x%02x\r\n", ret);
        return ret;
      }

    printf("BATTERY SERVICE COMPLETED!!!\r\n");
    return BLE_STATUS_SUCCESS;
}




/*DEVICE INFORMATION SERVICE AND CHAR BEGIN*/
uint16_t dif_service_handle;
uint16_t system_id_char_handle,
model_number_char_handle,
serial_number_char_handle,
firmware_number_char_handle,
hardware_number_char_handle,
software_number_char_handle,
manufacturer_name_char_handle,
ieee_certification_char_handle,
pnp_id_char_handle;
/**
 * @brief Adds DEVICE INFORMATION SERVICE (UUID 0x180A) with:
 * Characteristic (0x2A23) SYSTEM ID UUID
 * Characteristic (0x2A24) MODEL NUMBER UUID x
 * Characteristic (0x2A25) SERIAL NUMBER x
 * Characteristic (0x2A26) FIRMWARE REVISION x
 * Characteristic (0x2A27) HARDWARE REVISION x
 * Characteristic (0x2A28) SOFTWARE REVISION x
 * Characteristic (0x2A29) MANUFACTURER NAME x
 * Characteristic (0x2A2A) IEEE CERTIFICATION
 * Characteristic (0x2A50) PNP ID x
 *
 * calls updateDIService() to update some characteristics
 */
tBleStatus addDeviceInformationService(devInfService_Type* devInf){

    Service_UUID_t device_info_service_uuid;
    Char_UUID_t system_id_char_uuid,
    model_no_char_uuid,
    serial_no_char_uuid,
    firmware_no_char_uuid,
    hardware_no_char_uuid,
    software_no_char_uuid,
    manufacturer_name_char_uuid,
    ieee_cert_char_uuid, pnp_id_char_uuid;

    device_info_service_uuid.Service_UUID_16=DEVICE_INFORMATION_SERVICE_UUID;

    system_id_char_uuid.Char_UUID_16=SYSTEM_ID_UUID;
    model_no_char_uuid.Char_UUID_16=MODEL_NUMBER_UUID;
    serial_no_char_uuid.Char_UUID_16=SERIAL_NUMBER_UUID;
    firmware_no_char_uuid.Char_UUID_16=FIRMWARE_REVISION_UUID;
    hardware_no_char_uuid.Char_UUID_16=HARDWARE_REVISION_UUID;
    software_no_char_uuid.Char_UUID_16=SOFTWARE_REVISION_UUID;
    manufacturer_name_char_uuid.Char_UUID_16=MANUFACTURER_NAME_UUID;
    ieee_cert_char_uuid.Char_UUID_16=IEEE_CERTIFICATION_UUID;
    pnp_id_char_uuid.Char_UUID_16=PNP_ID_UUID;


    serviceMaxAttributeRecords = 26;
    ret = aci_gatt_add_service(UUID_TYPE_16,
                &device_info_service_uuid,
                PRIMARY_SERVICE,
                serviceMaxAttributeRecords,
                &dif_service_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
       printf("Error in Adding Device Information SERVICE 0x%02x\n", ret);
       return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &manufacturer_name_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &manufacturer_name_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (MANUFACTURER) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }



    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &system_id_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &system_id_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (SYSTEM ID) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }


    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &model_no_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &model_number_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (MODEL NO) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &serial_no_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &serial_number_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (SERIAL NO) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &firmware_no_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &firmware_number_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (FIRMWARE NO) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &hardware_no_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &hardware_number_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (HARDWARE NO) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }



    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &software_no_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &software_number_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (SOFTWARE NO) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }


    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &ieee_cert_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &ieee_certification_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (IEEE CERT) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(dif_service_handle,
                UUID_TYPE_16,
                &pnp_id_char_uuid,
                charValueLength,
                CHAR_PROP_READ,
                ATTR_PERMISSION_NONE,
                GATT_DONT_NOTIFY_EVENTS,
                encrypKeySize,
                CHAR_VALUE_LEN_CONSTANT,
                &pnp_id_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (PNP ID) for DEVICE INFORMATION SERVICE 0x%02x\n", ret);
        return ret;
    }


    updateDIService(devInf);
        return BLE_STATUS_SUCCESS;
}





uint16_t hid_service_handle;
uint16_t protocol_mode_char_handle,
input_report_char_handle, output_report_char_handle, feature_report_char_handle,
report_map_char_handle, boot_keyboard_input_char_handle,
boot_keyboard_output_char_handle,hid_info_char_handle, hid_control_point_char_handle;

uint16_t client_char_config_descriptor_handle,
report_reference_char_descriptor_handle_input,
report_reference_char_descriptor_handle_output,
report_reference_char_descriptor_handle_feature,
external_Report_Descriptor_handle;
/**
 * @brief Adds Human Interface Device SERVICE (UUID 0x1812) with:
 * Characteristic (0x2A4E) PROTOCOL MODE
 * Characteristic (0x2A4D) REPORT
 * Characteristic (0x2A4B) REPORT MAP (Input, Output, Feature)
 *              -Each has 1 (0x2908) Report Reference descriptor
 *              -Input has 1 (0x2902) Client Characteristic descriptor
 * Characteristic (0x2A22) BOOT KEYBOARD INPUT REPORT
 * Characteristic (0x2A32) BOOT KEYBOARD OUTPUT REPORT
 * Characteristic (0x2A4A) HID INFORMATION
 * Characteristic (0x2A4C) CONTROL POINT
 */
tBleStatus addHumanInterfaceService(hidService_Type* hid)
{
    Service_UUID_t hid_service_uuid;
    Char_UUID_t protocol_mode_char_uuid,
    input_report_char_uuid,
    output_report_char_uuid,
    feature_report_char_uuid,
    report_map_char_uuid,boot_keyboard_input_char_uuid, boot_keyboard_output_char_uuid,
    hid_information_char_uuid,hid_control_point_char_uuid;

    Char_Desc_Uuid_t report_reference_descriptor_input_uuid,
    report_reference_descriptor_output_uuid,
    report_reference_descriptor_feature_uuid,
    client_charac_config_descriptor_uuid;

    hid_service_uuid.Service_UUID_16=HUMAN_INTERFACE_DEVICE_SERVICE_UUID;

    protocol_mode_char_uuid.Char_UUID_16=PROTOCOL_MODE_CHAR_UUID;
    input_report_char_uuid.Char_UUID_16=REPORT_CHAR_UUID;
    output_report_char_uuid.Char_UUID_16=REPORT_CHAR_UUID;
    feature_report_char_uuid.Char_UUID_16=REPORT_CHAR_UUID;
    report_map_char_uuid.Char_UUID_16=REPORT_MAP_CHAR_UUID;
    boot_keyboard_input_char_uuid.Char_UUID_16=BOOT_KEYBOARD_INPUT_REPORT_CHAR_UUID;
    boot_keyboard_output_char_uuid.Char_UUID_16=BOOT_KEYBOARD_OUTPUT_REPORT_CHAR_UUID;
    hid_information_char_uuid.Char_UUID_16=HID_INFORMATION_CHAR_UUID;
    hid_control_point_char_uuid.Char_UUID_16=HID_CONTROL_POINT_CHAR_UUID;
    report_reference_descriptor_input_uuid.Char_UUID_16 = HID_REPORT_REFERENCE_CHAR_DESCRIPTOR;
    report_reference_descriptor_output_uuid.Char_UUID_16 = HID_REPORT_REFERENCE_CHAR_DESCRIPTOR;
    report_reference_descriptor_feature_uuid.Char_UUID_16 = HID_REPORT_REFERENCE_CHAR_DESCRIPTOR;
    client_charac_config_descriptor_uuid.Char_UUID_16 = HID_CLIENT_CHARACTERISTIC_CON_DESCRIPTOR;


    serviceMaxAttributeRecords = 26;
    ret = aci_gatt_add_service(UUID_TYPE_16,
                    &hid_service_uuid,
                    PRIMARY_SERVICE,
                    serviceMaxAttributeRecords,
                    &hid_service_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
       printf("Error in Adding Human Interface SERVICE 0x%02x\n", ret);
       return ret;
    }

    //INPUT REPORT
    charValueLength = 8;
        encrypKeySize = 10;
        ret = aci_gatt_add_char(hid_service_handle,
                            UUID_TYPE_16,
                            &input_report_char_uuid,
                            charValueLength,
                            CHAR_PROP_READ | CHAR_PROP_NOTIFY |CHAR_PROP_WRITE,
                            ATTR_PERMISSION_NONE,
            GATT_NOTIFY_ATTRIBUTE_WRITE,
                            encrypKeySize,
                            CHAR_VALUE_LEN_CONSTANT,
                            &input_report_char_handle);
        if (ret != BLE_STATUS_SUCCESS)
        {
            printf("Error in Adding Char (INPUT REPORT) for HID SERVICE 0x%02x\n", ret);
            return ret;
        }
//adding Client Characteristic Config Descriptor for input report
        ret = aci_gatt_add_char_desc(
                hid_service_handle,
                input_report_char_handle,
                UUID_TYPE_16,
                &client_charac_config_descriptor_uuid,
                5,
                5,
                0,
                ATTR_PERMISSION_NONE,
                ATTR_ACCESS_READ_ONLY,
                GATT_DONT_NOTIFY_EVENTS,
                10,
                CHAR_VALUE_LEN_CONSTANT,
                &client_char_config_descriptor_handle
        );

//adding report reference characteristic descriptor for INPUT REPORT
        uint8_t reportType[] = {1}; //input report
        ret = aci_gatt_add_char_desc(
                hid_service_handle,
                input_report_char_handle,
                UUID_TYPE_16,
                &report_reference_descriptor_input_uuid,
                5,
                5,
                reportType,
                ATTR_PERMISSION_NONE,
                ATTR_ACCESS_READ_ONLY,
                GATT_DONT_NOTIFY_EVENTS,
                10,
                CHAR_VALUE_LEN_CONSTANT,
                &report_reference_char_descriptor_handle_input
        );



        //OUTPUT REPORT
        charValueLength = 8;
            encrypKeySize = 10;
            ret = aci_gatt_add_char(hid_service_handle,
                                UUID_TYPE_16,
                                &output_report_char_uuid,
                                charValueLength,
                                CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP |CHAR_PROP_WRITE,
                                ATTR_PERMISSION_NONE,
                                GATT_NOTIFY_ATTRIBUTE_WRITE,
                                encrypKeySize,
                                CHAR_VALUE_LEN_CONSTANT,
                                &output_report_char_handle);
            if (ret != BLE_STATUS_SUCCESS)
            {
                printf("Error in Adding Char (OUTPUT REPORT) for HID SERVICE 0x%02x\n", ret);
                return ret;
            }



            //adding report reference characteristic descriptor for output REPORT
                    reportType[0] = 2; //output report
                    ret = aci_gatt_add_char_desc(
                            hid_service_handle,
                            input_report_char_handle,
                            UUID_TYPE_16,
                            &report_reference_descriptor_output_uuid,
                            5,
                            5,
                            reportType,
                            ATTR_PERMISSION_NONE,
                            ATTR_ACCESS_READ_ONLY,
                            GATT_NOTIFY_ATTRIBUTE_WRITE,
                            10,
                            CHAR_VALUE_LEN_CONSTANT,
                            &report_reference_char_descriptor_handle_output
                    );




            //FEATURE REPORT
            charValueLength = 8;
                encrypKeySize = 10;
                ret = aci_gatt_add_char(hid_service_handle,
                                    UUID_TYPE_16,
                                    &feature_report_char_uuid,
                                    charValueLength,
                                    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP |CHAR_PROP_WRITE,
                                    ATTR_PERMISSION_NONE,
                    GATT_NOTIFY_ATTRIBUTE_WRITE,
                                    encrypKeySize,
                                    CHAR_VALUE_LEN_CONSTANT,
                                    &feature_report_char_handle
                                    );
                if (ret != BLE_STATUS_SUCCESS)
                {
                    printf("Error in Adding Char (FEATURE REPORT) for HID SERVICE 0x%02x\n", ret);
                    return ret;
                }

                //adding report reference characteristic descriptor for feature REPORT
                        reportType[0] = 3; //feature report
                        ret = aci_gatt_add_char_desc(
                                hid_service_handle,
                                input_report_char_handle,
                                UUID_TYPE_16,
                                &report_reference_descriptor_feature_uuid,
                                5,
                                5,
                                reportType,
                                ATTR_PERMISSION_NONE,
                                ATTR_ACCESS_READ_ONLY,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &report_reference_char_descriptor_handle_feature
                        );


    charValueLength = 1;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(hid_service_handle,
                        UUID_TYPE_16,
                        &protocol_mode_char_uuid,
                        charValueLength,
                        CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP,
                        ATTR_PERMISSION_NONE,
                        GATT_NOTIFY_ATTRIBUTE_WRITE,
                        encrypKeySize,
                        CHAR_VALUE_LEN_CONSTANT,
                        &protocol_mode_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (PROTOCOL MODE) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }


    charValueLength = sizeof(reportDesc);
    encrypKeySize = 10;
    ret = aci_gatt_add_char(hid_service_handle,
                        UUID_TYPE_16,
                        &report_map_char_uuid,
                        charValueLength,
                        CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP,
                        ATTR_PERMISSION_NONE,
                        GATT_DONT_NOTIFY_EVENTS,
                        encrypKeySize,
                        CHAR_VALUE_LEN_CONSTANT,
                        &report_map_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (REPORT MAP) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }

    ret = aci_gatt_update_char_value_ext(
                0,
				hid_service_handle,
				report_map_char_handle,
                GATT_LOCAL_UPDATE,
				charValueLength,
                0,
				charValueLength,
				reportDesc);


    if (ret != BLE_STATUS_SUCCESS)
        {
            printf("ERROR  at updating report map: 0x%02x\n", ret);
            return ret;
        }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(hid_service_handle,
                        UUID_TYPE_16,
                        &boot_keyboard_input_char_uuid,
                        charValueLength,
                        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
                        ATTR_PERMISSION_NONE,
        GATT_NOTIFY_ATTRIBUTE_WRITE,
                        encrypKeySize,
                        CHAR_VALUE_LEN_CONSTANT,
                        &boot_keyboard_input_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (BOOT KEYBOARD INPUT) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(hid_service_handle,
                        UUID_TYPE_16,
                        &boot_keyboard_output_char_uuid,
                        charValueLength,
                        CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP,
                        ATTR_PERMISSION_NONE,
                        GATT_NOTIFY_ATTRIBUTE_WRITE,
                        encrypKeySize,
                        CHAR_VALUE_LEN_CONSTANT,
                        &boot_keyboard_output_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (BOOT KEYBOARD OUTPUT) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }

    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(hid_service_handle,
                        UUID_TYPE_16,
                        &hid_information_char_uuid,
                        charValueLength,
                        CHAR_PROP_READ,
                        ATTR_PERMISSION_NONE,
                        GATT_DONT_NOTIFY_EVENTS,
                        encrypKeySize,
                        CHAR_VALUE_LEN_CONSTANT,
                        &hid_info_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (HID INFORMATION) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }


    typedef struct _tHidInfoChar
    {
        /**
         * 16-bit unsigned integer representing version
         * number of base USB HID Specification
         * implemented by HID Device
         */
        uint16_t bcdHID;

        /**
         * 8-bit integer identifying country HID Device
         * hardware is localized for. Most
         * hardware is not localized (value 0x00)
         */
        uint8_t bCountryCode;

        /**
         * Bit 0: RemoteWake - Boolean value indicating whether
         * HID Device is capable of sending a wake-signal to a HID Host.\n
         * 0bXXXXXXX0 = FALSE\n
         * 0bXXXXXXX1 = TRUE\n
         * Bit 1: NormallyConnectable - Boolean value indicating
         * whether HID Device will be advertising when bonded but not connected.
         * 0bXXXXXX0X = FALSE\n
         * 0xXXXXXX1X = TRUE
        */
        uint8_t flags;
    }tHidInfoChar;




    //uint8_t dataBuffer[] = { 0xDB,0xFA, 90,0x02 };

    tHidInfoChar appHidServData;
    
    appHidServData.bcdHID = 0x0111;
    appHidServData.bCountryCode = 0x00;
    appHidServData.flags = 0x02;
 
    ret = aci_gatt_update_char_value(hid_service_handle, hid_info_char_handle, 0, sizeof(appHidServData), (uint8_t*)&appHidServData);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding VALUE for Char (HID INFORMATION) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }


    charValueLength = 20;
    encrypKeySize = 10;
    ret = aci_gatt_add_char(hid_service_handle,
                        UUID_TYPE_16,
                        &hid_control_point_char_uuid,
                        charValueLength,
                        CHAR_PROP_WRITE_WITHOUT_RESP,
                        ATTR_PERMISSION_NONE,
        GATT_NOTIFY_ATTRIBUTE_WRITE,
                        encrypKeySize,
                        CHAR_VALUE_LEN_CONSTANT,
                        &hid_control_point_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Adding Char (HID CONTROL POINT) for HID SERVICE 0x%02x\n", ret);
        return ret;
    }

   // updateHIDServiceParams();

    return BLE_STATUS_SUCCESS;
}



/*
 * Updates the values for the caracteristics of
 * Human Interface Devservice (global service and characteristics handles)
 */
/*
void updateHIDServiceParams()
{

    hidService_Type hid_param;
    hidService_Type hid;

    hid_param.bootSupport = TRUE;
    hid_param.reportSupport = TRUE;
    hid_param.reportReferenceDesc[0].ID = REPORT_ID;
    hid_param.reportReferenceDesc[0].type = INPUT_REPORT;
    hid_param.reportReferenceDesc[0].length = 8;
    hid_param.reportReferenceDesc[1].ID = REPORT_ID;
    hid_param.reportReferenceDesc[1].type = OUTPUT_REPORT;
    hid_param.reportReferenceDesc[1].length = 1;
    hid_param.isBootDevKeyboard = TRUE;
    hid_param.isBootDevMouse = FALSE;
    hid_param.externalReportEnabled = 1;
    hid_param.includedServiceEnabled = FALSE;
    hid_param.informationCharac[0] = 0x01;
    hid_param.informationCharac[1] = 0x01;
    hid_param.informationCharac[2] = 0;
    hid_param.informationCharac[3] = 0x01;

    hid = hid_param;

    hid.reportDescLen = sizeof(reportDesc);
    hid.reportDesc = reportDesc;



}
*/





/*
 * Updates the values for the caracteristics of
 * Device Information service (global service and characteristics handles)
 * Manufacturer Name
 * Model Number
 * Firmware Revision
 * Software Revision
 * PnP ID
 */
void updateDIService(devInfService_Type* devInf)
{
      memcpy(devInf->manufacName, "ST Micro ", MANUFAC_NAME_LEN);
      memcpy(devInf->modelNumber, "0001", MODEL_NUMB_LEN);
      memcpy(devInf->fwRevision, "0630", FW_REV_LEN);
      memcpy(devInf->swRevision, "0001", SW_REV_LEN);
      devInf->pnpID[0] = 0x01;
      devInf->pnpID[1] = 0x30;
      devInf->pnpID[2] = 0x00;
      devInf->pnpID[3] = 0xfc;
      devInf->pnpID[4] = 0x00;
      devInf->pnpID[5] = 0xec;
      devInf->pnpID[6] = 0x00;

/*
      uint16_t system_id_char_handle,
      model_number_char_handle,
      serial_number_char_handle,
      firmware_number_char_handle,
      hardware_number_char_handle,
      software_number_char_handle,
      manufacturer_name_char_handle,
      ieee_certification_char_handle,
      pnp_id_char_handle;
*/
      ret = aci_gatt_update_char_value_ext(
            0,
            dif_service_handle,
            manufacturer_name_char_handle,
            GATT_LOCAL_UPDATE,
            20,
            0,
            sizeof(devInf->manufacName),
            devInf->manufacName);

      if(ret != BLE_STATUS_SUCCESS)
      {
        printf("Failed to update (MANUFACT NAME): 0x%02x\r\n", ret);
      }

      ret = aci_gatt_update_char_value_ext(
            0,
            dif_service_handle,
            model_number_char_handle,
            GATT_LOCAL_UPDATE,
            20,
            0,
            sizeof(devInf->modelNumber),
            devInf->modelNumber);

      if(ret != BLE_STATUS_SUCCESS)
      {
        printf("Failed to update (MODEL NUMBER): 0x%02x\r\n", ret);
      }


      ret = aci_gatt_update_char_value_ext(
            0,
            dif_service_handle,
            firmware_number_char_handle,
            GATT_LOCAL_UPDATE,
            20,
            0,
            sizeof(devInf->fwRevision),
            devInf->fwRevision);

      if(ret != BLE_STATUS_SUCCESS)
      {
        printf("Failed to update (FIRMWARE NUMBER): 0x%02x\r\n", ret);
      }


      ret = aci_gatt_update_char_value_ext(
            0,
            dif_service_handle,
            software_number_char_handle,
            GATT_LOCAL_UPDATE,
            20,
            0,
            sizeof(devInf->swRevision),
            devInf->swRevision);

      if(ret != BLE_STATUS_SUCCESS)
      {
        printf("Failed to update (SOFTWARE NUMBER): 0x%02x\r\n", ret);
      }

      ret = aci_gatt_update_char_value_ext(
            0,
            dif_service_handle,
            pnp_id_char_handle,
            GATT_LOCAL_UPDATE,
            20,
            0,
            sizeof(devInf->pnpID),
            devInf->pnpID);

      if(ret != BLE_STATUS_SUCCESS)
      {
        printf("Failed to update (PnP ID): 0x%02x\r\n", ret);
      }
}



/*


void hci_le_connection_complete_event
                                    (uint8_t Status,
                                    uint16_t Connection_Handle,
                                    uint8_t Role,
                                    uint8_t Peer_Address_Type,
                                    uint8_t Peer_Address[6],
                                    uint16_t Conn_Interval,
                                    uint16_t Conn_Latency,
                                    uint16_t Supervision_Timeout,
                                    uint8_t Master_Clock_Accuracy
                                    )

    {
    connected=1;
    connection_handle=Connection_Handle;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    printf("Connected \r\n");
    }
*/
/*
void hci_le_disconnection_complete_event
                                        (uint8_t Status,
                                         uint16_t Connection_Handle,
                                         uint8_t Reason
                                        )
    {
    connected=0;
    set_connectable=1;
    Connection_Handle=0;
    printf("Disconnected \r\n");
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
    */

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


/*******************************************************************************
* Function Name  : Find_DeviceName.
* Description    : Extracts the device name.
* Input          : Data length.
*                  Data value
* Return         : TRUE if the local name found is the expected one, FALSE otherwise.
*******************************************************************************/

extern uint8_t local_name[];
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











