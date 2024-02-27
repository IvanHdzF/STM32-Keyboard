#include "services.h"
#include "bluenrg1_aci.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_hci_le.h"
#include "bluenrg1_types.h"
#include "bluenrg_conf.h"
#include "main.h"
#include "stdio.h"
#include "stdlib.h"

// Un-comment the following macro if you want to use the Android/iOS User App
// #define USER_APP
#ifdef USER_APP
#define ATTR_PERMISSION ATTR_PERMISSION_NONE
#else
#define ATTR_PERMISSION                                                        \
  (ATTR_PERMISSION_ENCRY_READ | ATTR_PERMISSION_ENCRY_WRITE)
#endif

/*Global Variables*/
uint16_t battery_service_handle;
uint16_t battery_level_char_handle;
uint16_t BATTERY_client_char_config_desc;

uint16_t dif_service_handle;
uint16_t system_id_char_handle, model_number_char_handle,
    serial_number_char_handle, firmware_number_char_handle,
    hardware_number_char_handle, software_number_char_handle,
    manufacturer_name_char_handle, ieee_certification_char_handle,
    pnp_id_char_handle;

uint16_t hid_service_handle;
uint16_t protocol_mode_char_handle, input_report_char_handle,
    output_report_char_handle, feature_report_char_handle,
    report_map_char_handle, boot_keyboard_input_char_handle,
    boot_keyboard_output_char_handle, hid_info_char_handle,
    hid_control_point_char_handle;

uint16_t client_char_config_descriptor_handle,
    report_reference_char_descriptor_handle_input,
    report_reference_char_descriptor_handle_output,
    report_reference_char_descriptor_handle_feature,
    external_Report_Descriptor_handle;

uint8_t serviceMaxAttributeRecords, encrypKeySize;
uint16_t charValueLength;

extern uint8_t local_name[];

// Keyboard report descriptor
uint8_t reportDesc[] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xA1, 0x01, // COLLECTION (Application)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    // 0x85, 0x01,   // REPORT_ID (1)
    //  1 byte Modifier: Ctrl, Shift and other modifier keys, 8 in total
    0x19, 0xE0, // USAGE_MINIMUM (kbd LeftControl)
    0x29, 0xE7, // USAGE_MAXIMUM (kbd Right GUI)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x75, 0x01, // REPORT_SIZE (1)
    0x95, 0x08, // REPORT_COUNT (8)
    0x81, 0x02, // INPUT (Data,Var,Abs)
    // 1 Reserved byte
    0x95, 0x01, // REPORT_COUNT (1)
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
    0xC0,       // END_COLLECTION
};

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
uint8_t hidAddServices(devInfService_Type *devInf) {

  uint8_t ret;

  ret = addBatteryService();
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("ERROR IN ADDING BATTERY SERVICE\r\n");
    return ret;
  }

  ret = addDeviceInformationService(devInf);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("ERROR IN ADDING DEVICE INFORMATION SERVICE\r\n");
    return ret;
  }

  ret = addHumanInterfaceService();
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("ERROR IN ADDING HUMAN INTERFACE SERVICE\r\n");
    return ret;
  }

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief Adds BATTERY SERVICE (UUID 0x180F) with:
 * 1 characteristic (BATTERY LEVEL UUID 0x2A19)
 * Updates the value for BATTERY LEVEL to 50
 */
tBleStatus addBatteryService() {

  uint8_t ret;
  Service_UUID_t battery_service_uuid;
  Char_UUID_t battery_level_char_uuid;
  // Char_Desc_Uuid_t char_presentation_format_descriptor;

  battery_service_uuid.Service_UUID_16 = BATTERY_SERVICE_SERVICE_UUID;
  battery_level_char_uuid.Char_UUID_16 = BATTERY_LEVEL_CHAR_UUID;

  serviceMaxAttributeRecords = 4;

  ret =
      aci_gatt_add_service(UUID_TYPE_16, &battery_service_uuid, PRIMARY_SERVICE,
                           serviceMaxAttributeRecords, &battery_service_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding BATTERY SERVICE 0x%02x\n", ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(battery_service_handle, UUID_TYPE_16,
                          &battery_level_char_uuid, charValueLength,
                          CHAR_PROP_READ, ATTR_PERMISSION,
                          GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize,
                          CHAR_VALUE_LEN_CONSTANT, &battery_level_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char for BATTERY SERVICE 0x%02x\n", ret);
    return ret;
  }
  uint8_t batteryLevel = 75;
  ret = aci_gatt_update_char_value_ext(
      0, battery_service_handle, battery_level_char_handle, 0,
      sizeof(batteryLevel), 0, sizeof(batteryLevel), (uint8_t *)&batteryLevel);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Updating BATERRY level failed failed: 0x%02x\r\n", ret);
    return ret;
  }

  BLUENRG_PRINTF("BATTERY SERVICE COMPLETED!!!\r\n");
  return BLE_STATUS_SUCCESS;
}

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
tBleStatus addDeviceInformationService(devInfService_Type *devInf) {

  uint8_t ret;

  Service_UUID_t device_info_service_uuid;
  Char_UUID_t system_id_char_uuid, model_no_char_uuid, serial_no_char_uuid,
      firmware_no_char_uuid, hardware_no_char_uuid, software_no_char_uuid,
      manufacturer_name_char_uuid, ieee_cert_char_uuid, pnp_id_char_uuid;

  device_info_service_uuid.Service_UUID_16 = DEVICE_INFORMATION_SERVICE_UUID;

  system_id_char_uuid.Char_UUID_16 = SYSTEM_ID_UUID;
  model_no_char_uuid.Char_UUID_16 = MODEL_NUMBER_UUID;
  serial_no_char_uuid.Char_UUID_16 = SERIAL_NUMBER_UUID;
  firmware_no_char_uuid.Char_UUID_16 = FIRMWARE_REVISION_UUID;
  hardware_no_char_uuid.Char_UUID_16 = HARDWARE_REVISION_UUID;
  software_no_char_uuid.Char_UUID_16 = SOFTWARE_REVISION_UUID;
  manufacturer_name_char_uuid.Char_UUID_16 = MANUFACTURER_NAME_UUID;
  ieee_cert_char_uuid.Char_UUID_16 = IEEE_CERTIFICATION_UUID;
  pnp_id_char_uuid.Char_UUID_16 = PNP_ID_UUID;

  serviceMaxAttributeRecords = 26;
  ret = aci_gatt_add_service(UUID_TYPE_16, &device_info_service_uuid,
                             PRIMARY_SERVICE, serviceMaxAttributeRecords,
                             &dif_service_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Device Information SERVICE 0x%02x\n", ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &manufacturer_name_char_uuid,
      charValueLength, CHAR_PROP_READ, ATTR_PERMISSION_NONE,
      GATT_DONT_NOTIFY_EVENTS, encrypKeySize, CHAR_VALUE_LEN_CONSTANT,
      &manufacturer_name_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (MANUFACTURER) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &system_id_char_uuid, charValueLength,
      CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &system_id_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (SYSTEM ID) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(dif_service_handle, UUID_TYPE_16, &model_no_char_uuid,
                          charValueLength, CHAR_PROP_READ, ATTR_PERMISSION_NONE,
                          GATT_DONT_NOTIFY_EVENTS, encrypKeySize,
                          CHAR_VALUE_LEN_CONSTANT, &model_number_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (MODEL NO) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &serial_no_char_uuid, charValueLength,
      CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &serial_number_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (SERIAL NO) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &firmware_no_char_uuid, charValueLength,
      CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &firmware_number_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (FIRMWARE NO) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &hardware_no_char_uuid, charValueLength,
      CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &hardware_number_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (HARDWARE NO) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &software_no_char_uuid, charValueLength,
      CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &software_number_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (SOFTWARE NO) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      dif_service_handle, UUID_TYPE_16, &ieee_cert_char_uuid, charValueLength,
      CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &ieee_certification_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (IEEE CERT) for DEVICE INFORMATION "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(dif_service_handle, UUID_TYPE_16, &pnp_id_char_uuid,
                          charValueLength, CHAR_PROP_READ, ATTR_PERMISSION_NONE,
                          GATT_DONT_NOTIFY_EVENTS, encrypKeySize,
                          CHAR_VALUE_LEN_CONSTANT, &pnp_id_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (PNP ID) for DEVICE INFORMATION SERVICE 0x%02x\n",
        ret);
    return ret;
  }

  updateDIService(devInf);
  BLUENRG_PRINTF("DEVICE INFORMATION SERVICE COMPLETED!!!\r\n");
  return BLE_STATUS_SUCCESS;
}

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
tBleStatus addHumanInterfaceService() {
  uint8_t ret;
  Service_UUID_t hid_service_uuid;
  Char_UUID_t protocol_mode_char_uuid, input_report_char_uuid,
      output_report_char_uuid, feature_report_char_uuid, report_map_char_uuid,
      boot_keyboard_input_char_uuid, boot_keyboard_output_char_uuid,
      hid_information_char_uuid, hid_control_point_char_uuid;

  Char_Desc_Uuid_t report_reference_descriptor_input_uuid,
      report_reference_descriptor_output_uuid,
      report_reference_descriptor_feature_uuid,
      client_charac_config_descriptor_uuid;

  hid_service_uuid.Service_UUID_16 = HUMAN_INTERFACE_DEVICE_SERVICE_UUID;

  protocol_mode_char_uuid.Char_UUID_16 = PROTOCOL_MODE_CHAR_UUID;
  input_report_char_uuid.Char_UUID_16 = REPORT_CHAR_UUID;
  output_report_char_uuid.Char_UUID_16 = REPORT_CHAR_UUID;
  feature_report_char_uuid.Char_UUID_16 = REPORT_CHAR_UUID;
  report_map_char_uuid.Char_UUID_16 = REPORT_MAP_CHAR_UUID;
  boot_keyboard_input_char_uuid.Char_UUID_16 =
      BOOT_KEYBOARD_INPUT_REPORT_CHAR_UUID;
  boot_keyboard_output_char_uuid.Char_UUID_16 =
      BOOT_KEYBOARD_OUTPUT_REPORT_CHAR_UUID;
  hid_information_char_uuid.Char_UUID_16 = HID_INFORMATION_CHAR_UUID;
  hid_control_point_char_uuid.Char_UUID_16 = HID_CONTROL_POINT_CHAR_UUID;
  report_reference_descriptor_input_uuid.Char_UUID_16 =
      HID_REPORT_REFERENCE_CHAR_DESCRIPTOR;
  report_reference_descriptor_output_uuid.Char_UUID_16 =
      HID_REPORT_REFERENCE_CHAR_DESCRIPTOR;
  report_reference_descriptor_feature_uuid.Char_UUID_16 =
      HID_REPORT_REFERENCE_CHAR_DESCRIPTOR;
  client_charac_config_descriptor_uuid.Char_UUID_16 =
      HID_CLIENT_CHARACTERISTIC_CON_DESCRIPTOR;

  serviceMaxAttributeRecords = 26;
  ret = aci_gatt_add_service(UUID_TYPE_16, &hid_service_uuid, PRIMARY_SERVICE,
                             serviceMaxAttributeRecords, &hid_service_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Human Interface SERVICE 0x%02x\n", ret);
    return ret;
  }

  // INPUT REPORT
  charValueLength = 8;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &input_report_char_uuid,
      charValueLength, CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE,
      ATTR_PERMISSION, GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize,
      CHAR_VALUE_LEN_CONSTANT, &input_report_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (INPUT REPORT) for HID SERVICE 0x%02x\n", ret);
    return ret;
  }
  // adding Client Characteristic Config Descriptor for input report
  ret = aci_gatt_add_char_desc(
      hid_service_handle, input_report_char_handle, UUID_TYPE_16,
      &client_charac_config_descriptor_uuid, 5, 5, 0, ATTR_PERMISSION,
      ATTR_ACCESS_READ_ONLY, GATT_DONT_NOTIFY_EVENTS, 10,
      CHAR_VALUE_LEN_CONSTANT, &client_char_config_descriptor_handle);

  // adding report reference characteristic descriptor for INPUT REPORT
  uint8_t reportType[2] = {0, 1}; // input report
  ret = aci_gatt_add_char_desc(
      hid_service_handle, input_report_char_handle, UUID_TYPE_16,
      &report_reference_descriptor_input_uuid, 5, 2, reportType,
      ATTR_PERMISSION, ATTR_ACCESS_READ_ONLY, GATT_DONT_NOTIFY_EVENTS, 10,
      CHAR_VALUE_LEN_CONSTANT, &report_reference_char_descriptor_handle_input);

  // OUTPUT REPORT
  charValueLength = 8;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &output_report_char_uuid,
      charValueLength,
      CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE,
      ATTR_PERMISSION, GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize,
      CHAR_VALUE_LEN_CONSTANT, &output_report_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (OUTPUT REPORT) for HID SERVICE 0x%02x\n", ret);
    return ret;
  }

  // adding report reference characteristic descriptor for output REPORT
  reportType[1] = 2; // output report
  ret = aci_gatt_add_char_desc(
      hid_service_handle, input_report_char_handle, UUID_TYPE_16,
      &report_reference_descriptor_output_uuid, 5, 2, reportType,
      ATTR_PERMISSION, ATTR_ACCESS_READ_ONLY, GATT_NOTIFY_ATTRIBUTE_WRITE, 10,
      CHAR_VALUE_LEN_CONSTANT, &report_reference_char_descriptor_handle_output);

  // FEATURE REPORT
  charValueLength = 8;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &feature_report_char_uuid,
      charValueLength,
      CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_WRITE,
      ATTR_PERMISSION, GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize,
      CHAR_VALUE_LEN_CONSTANT, &feature_report_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (FEATURE REPORT) for HID SERVICE 0x%02x\n", ret);
    return ret;
  }

  // adding report reference characteristic descriptor for feature REPORT
  reportType[1] = 3; // feature report
  ret = aci_gatt_add_char_desc(
      hid_service_handle, input_report_char_handle, UUID_TYPE_16,
      &report_reference_descriptor_feature_uuid, 5, 2, reportType,
      ATTR_PERMISSION, ATTR_ACCESS_READ_ONLY, GATT_DONT_NOTIFY_EVENTS, 10,
      CHAR_VALUE_LEN_CONSTANT,
      &report_reference_char_descriptor_handle_feature);

  charValueLength = 1;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &protocol_mode_char_uuid,
      charValueLength, CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP,
      ATTR_PERMISSION, GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize,
      CHAR_VALUE_LEN_CONSTANT, &protocol_mode_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (PROTOCOL MODE) for HID SERVICE 0x%02x\n", ret);
    return ret;
  }

  charValueLength = sizeof(reportDesc);
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &report_map_char_uuid, charValueLength,
      CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION,
      GATT_DONT_NOTIFY_EVENTS, encrypKeySize, CHAR_VALUE_LEN_CONSTANT,
      &report_map_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding Char (REPORT MAP) for HID SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  ret = aci_gatt_update_char_value_ext(
      0, hid_service_handle, report_map_char_handle, GATT_LOCAL_UPDATE,
      charValueLength, 0, charValueLength, reportDesc);

  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("ERROR  at updating report map: 0x%02x\n", ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &boot_keyboard_input_char_uuid,
      charValueLength, CHAR_PROP_READ | CHAR_PROP_NOTIFY, ATTR_PERMISSION,
      GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize, CHAR_VALUE_LEN_CONSTANT,
      &boot_keyboard_input_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (BOOT KEYBOARD INPUT) for HID SERVICE 0x%02x\n",
        ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &boot_keyboard_output_char_uuid,
      charValueLength,
      CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP,
      ATTR_PERMISSION, GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize,
      CHAR_VALUE_LEN_CONSTANT, &boot_keyboard_output_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (BOOT KEYBOARD OUTPUT) for HID SERVICE 0x%02x\n",
        ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &hid_information_char_uuid,
      charValueLength, CHAR_PROP_READ, ATTR_PERMISSION, GATT_DONT_NOTIFY_EVENTS,
      encrypKeySize, CHAR_VALUE_LEN_CONSTANT, &hid_info_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (HID INFORMATION) for HID SERVICE 0x%02x\n", ret);
    return ret;
  }

  typedef struct _tHidInfoChar {
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
  } tHidInfoChar;

  // uint8_t dataBuffer[] = { 0xDB,0xFA, 90,0x02 };

  tHidInfoChar appHidServData;

  appHidServData.bcdHID = 0x0111;
  appHidServData.bCountryCode = 0x00;
  appHidServData.flags = 0x02;

  ret = aci_gatt_update_char_value(hid_service_handle, hid_info_char_handle, 0,
                                   sizeof(appHidServData),
                                   (uint8_t *)&appHidServData);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Error in Adding VALUE for Char (HID INFORMATION) for HID "
                   "SERVICE 0x%02x\n",
                   ret);
    return ret;
  }

  charValueLength = 20;
  encrypKeySize = 10;
  ret = aci_gatt_add_char(
      hid_service_handle, UUID_TYPE_16, &hid_control_point_char_uuid,
      charValueLength, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION,
      GATT_NOTIFY_ATTRIBUTE_WRITE, encrypKeySize, CHAR_VALUE_LEN_CONSTANT,
      &hid_control_point_char_handle);
  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF(
        "Error in Adding Char (HID CONTROL POINT) for HID SERVICE 0x%02x\n",
        ret);
    return ret;
  }

  // updateHIDServiceParams();

  return BLE_STATUS_SUCCESS;
}

/*
 * Updates the values for the caracteristics of
 * Device Information service (global service and characteristics handles)
 * Manufacturer Name
 * Model Number
 * Firmware Revision
 * Software Revision
 * PnP ID
 */
void updateDIService(devInfService_Type *devInf) {
  uint8_t ret;
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

  ret = aci_gatt_update_char_value_ext(
      0, dif_service_handle, manufacturer_name_char_handle, GATT_LOCAL_UPDATE,
      20, 0, sizeof(devInf->manufacName), devInf->manufacName);

  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Failed to update (MANUFACT NAME): 0x%02x\r\n", ret);
  }

  ret = aci_gatt_update_char_value_ext(
      0, dif_service_handle, model_number_char_handle, GATT_LOCAL_UPDATE, 20, 0,
      sizeof(devInf->modelNumber), devInf->modelNumber);

  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Failed to update (MODEL NUMBER): 0x%02x\r\n", ret);
  }

  ret = aci_gatt_update_char_value_ext(
      0, dif_service_handle, firmware_number_char_handle, GATT_LOCAL_UPDATE, 20,
      0, sizeof(devInf->fwRevision), devInf->fwRevision);

  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Failed to update (FIRMWARE NUMBER): 0x%02x\r\n", ret);
  }

  ret = aci_gatt_update_char_value_ext(
      0, dif_service_handle, software_number_char_handle, GATT_LOCAL_UPDATE, 20,
      0, sizeof(devInf->swRevision), devInf->swRevision);

  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Failed to update (SOFTWARE NUMBER): 0x%02x\r\n", ret);
  }

  ret = aci_gatt_update_char_value_ext(
      0, dif_service_handle, pnp_id_char_handle, GATT_LOCAL_UPDATE, 20, 0,
      sizeof(devInf->pnpID), devInf->pnpID);

  if (ret != BLE_STATUS_SUCCESS) {
    BLUENRG_PRINTF("Failed to update (PnP ID): 0x%02x\r\n", ret);
  }
}
