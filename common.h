/*
 * Copyright 2017, Sierra Telecom. All Rights Reserved.
 *
 * This software, associated documentation and materials ("Software"),
 * is owned by Sierra Telecom ("Sierra") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Sierra
 * reserves the right to make changes to the Software without notice. Sierra
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Sierra does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Sierra product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Sierra's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Sierra against all liability.
 */

/** @file .h
 *
 *  Created on: October 14, 2017
 *      Author: greg.phillips
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include "wiced_apps_common.h"

/*
 *	Defines for common structures and defines with iMatrix system and Interface used by host App
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef USE_CCMRAM
#define CCMSRAM_ENABLED
#endif

#ifdef CCMSRAM_ENABLED
//#pragma message "CCMSRAM Enabled"
#define CCMSRAM __attribute__((section(".ccmram")))
#else
#define CCMSRAM
#endif

/******************************************************
 *                    Constants
 ******************************************************/
#define IMX_VERSION_FORMAT              "%d.%03d.%03d"
#define IMX_CONTROL_SENSOR_NAME_LENGTH  ( 32 )
#define IMX_CONTROL_NAME_LENGTH         (IMX_CONTROL_SENSOR_NAME_LENGTH)
/*
 * Product Capabilities
 */
#define IMX_WIFI_2_4GHZ                 0x00000001
#define IMX_WIFI_5_2GHZ                 0x00000002
#define IMX_WIFI_5_4GHZ                 0x00000003
#define IMX_WIFI_5_8GHZ                 0x00000008
#define IMX_BLUETOOTH_CLASSIC           0x00000100
#define IMX_BLUETOOTH_LOW_ENERGY        0x00000200
#define IMX_BLUETOOTH_MESH_1            0x00000400
#define IMX_BLUETOOTH_MESH_2            0x00000800

#define IMX_MAGIC_CONFIG                ( 0x87654321 )
#define IMX_PRODUCT_NAME_LENGTH         ( 64 )
#define IMX_DEVICE_NAME_LENGTH          ( 16 )
#define IMX_DEVICE_SERIAL_NUMBER_LENGTH ( 10 )
#define IMX_SAMPLE_LENGTH               ( 4 )
#define IMX_MAX_NO_BLE_DEVICES          ( 20 )
#define IMX_MAC_ADDRESS_LENGTH          ( 6 )
#define IMX_NO_RF_SCAN_RECORDS          ( 20 )
#define IMX_ORGANIZATION_ID_LENGTH      ( 10 )
#define IMX_PRODUCT_ID_LENGTH           ( 10 )
#define IMX_MAX_VAR_LENGTH_POOLS        ( 8 )
/*
 * Wi-Fi Credentials length
 */
#define IMX_WPA2PSK_LENGTH              ( 63 )
#define IMX_SSID_LENGTH                 ( 32 )
#define IMX_PASSWORD_LENGTH             ( 32 )
#define IMX_IMATRIX_URL_LENGTH          ( 64 )
#define IMX_IMATRIX_URI_LENGTH          ( 64 )
#define IMX_IMATRIX_SITE_LENGTH         ( 64 )

/*
 * Options are
 *
    WICED_SECURITY_OPEN
    WICED_SECURITY_WEP_PSK
    WICED_SECURITY_WEP_SHARED
    WICED_SECURITY_WPA_TKIP_PSK
    WICED_SECURITY_WPA_AES_PSK
    WICED_SECURITY_WPA_MIXED_PSK
    WICED_SECURITY_WPA2_AES_PSK
    WICED_SECURITY_WPA2_TKIP_PSK
    WICED_SECURITY_WPA2_MIXED_PSK

    WICED_SECURITY_WPA_TKIP_ENT
    WICED_SECURITY_WPA_AES_ENT
    WICED_SECURITY_WPA_MIXED_ENT
    WICED_SECURITY_WPA2_TKIP_ENT
    WICED_SECURITY_WPA2_AES_ENT
    WICED_SECURITY_WPA2_MIXED_ENT

    WICED_SECURITY_IBSS_OPEN
    WICED_SECURITY_WPS_OPEN
    WICED_SECURITY_WPS_SECURE

 *
 */

#define IMX_SEC_IN_DAY                  ( 24UL * 60UL * 60UL )
/*
 * Location defaults - Zephyr Cove Office
 */
#define IMX_FACTORY_LONGITUDE_DEFAULT           -119.943016
#define IMX_FACTORY_LATITUDE_DEFAULT            38.986835
#define IMX_FACTORY_ELEVATION_DEFAULT           1925.15         // Elevation in Meters

#define WARNING_LEVELS                  ( 3 )
#define IMX_NO_LEDS                     ( 3 )
/*
 * LED Settings
 */
#define IMX_LED_BLINK_1_1               0x0001  // Time in 100 mS intervals
#define IMX_LED_BLINK_1_2               0x0002
#define IMX_LED_BLINK_1_3               0x0003
#define IMX_LED_BLINK_1_4               0x0004
#define IMX_LED_BLINK_1_5               0x0005
#define IMX_LED_BLINK_1_6               0x0006
#define IMX_LED_BLINK_1_7               0x0007
#define IMX_LED_BLINK_1_8               0x0008
#define IMX_LED_BLINK_1_9               0x0009
#define IMX_LED_BLINK_1_10              0x000A
#define IMX_LED_BLINK_1_MASK            0x000F  // Time in 100 mS intervals
#define IMX_LED_BLINK_2_1               0x0010
#define IMX_LED_BLINK_2_2               0x0020
#define IMX_LED_BLINK_2_3               0x0030
#define IMX_LED_BLINK_2_4               0x0040
#define IMX_LED_BLINK_2_5               0x0050
#define IMX_LED_BLINK_2_6               0x0060
#define IMX_LED_BLINK_2_7               0x0070
#define IMX_LED_BLINK_2_8               0x0080
#define IMX_LED_BLINK_2_9               0x0090
#define IMX_LED_BLINK_2_10              0x00A0
#define IMX_LED_BLINK_2_MASK            0x00F0
#define IMX_LED_FLASH_1                 0x0100  // Time in 1 Sec intervals
#define IMX_LED_FLASH_2                 0x0200
#define IMX_LED_FLASH_3                 0x0300
#define IMX_LED_FLASH_4                 0x0400
#define IMX_LED_FLASH_5                 0x0500
#define IMX_LED_FLASH_6                 0x0600
#define IMX_LED_FLASH_7                 0x0700
#define IMX_LED_FLASH_8                 0x0800
#define IMX_LED_FLASH_9                 0x0900
#define IMX_LED_FLASH_10                0x0A00
#define IMX_LED_FLASH_MASK              0x0F00
#define IMX_LED_BLINK_1                 0x1000  // Master Blinking LED
#define IMX_LED_BLINK_2                 0x2000  // Slave Blinking LED
#define IMX_LED_FLASH                   0x4000  // Indicate this is a flash, this is a 1 Second event For Dual LEDs First LED ON for BLINK 1 second is on for BLINK 2 - off for remainder of 1 Second
#define IMX_LED_ALTERNATE               0x8000  // Alternate Blinking / Flashing

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
/*
 * iMatrix Wi Fi Mode
 */
typedef enum imx_wifi_mode {
    IMX_WIFI_ACCESS_POINT = 0,
    IMX_WIFI_STATION,
    IMX_WIFI_AD_HOC,
    IMX_WIFI_OFFLINE,
} imx_wifi_mode_t;
/*
 * Define Control and Sensor Errors
 */
typedef enum imx_result {
    IMX_SUCCESS = 0,
    IMX_INVALID_ENTRY,
    IMX_CONTROL_DISABLED,
    IMX_SENSOR_DISABLED,
    IMX_SENSOR_ERROR,
    IMX_SENSOR_INIT_ERROR,
    IMX_GENERAL_FAILURE,
    IMX_INIT_ERROR,
    IMX_I2C_ERROR,
    IMX_SPI_ERROR,
    IMX_INVALID_READING,
    IMX_FAIL_CRC,
    IMX_FAIL_COAP_SETUP,
    IMX_OUT_OF_MEMORY,
    IMX_MAXIMUM_CONTROLS_EXCEEDED,
    IMX_MAXIMUM_SENSORS_EXCEEDED,
    IMX_MAXIMUM_HISTORY_EXCEEDED,
    IMX_MAXIMUM_VARIBALE_DATA_EXCEEDED,
    IMX_MUST_SUPPLY_DEFAULTS,
    IMX_MUST_SUPPLY_CONTROL,
    IMX_MUST_SUPPLY_SAMPLE,
    /*
     * Following are Result/Error Conditions for Sensor Sampling
     */
    IMX_NO_DATA,
    IMX_ON_BOARD_TEMP_ERROR,
    IMX_INTERNAL_ADC_ERROR,
} imx_result_t;
/*
 * Define Peripheral types
 */
typedef enum imx_peripheral_type {
    IMX_CONTROLS = 0,
    IMX_SENSORS,
    IMX_NO_PERIPHERAL_TYPES,
} imx_peripheral_type_t;
/*
 * Define data types for Controls & Sensors
 */
typedef enum {
    IMX_UINT32 = 0,
    IMX_INT32,
    IMX_FLOAT,
    IMX_VARIABLE_LENGTH,
    IMX_NO_DATA_TYPES,
} imx_data_types_t;
/*
 * Tsunami Warning codes
 */
typedef enum {
    IMX_INFORMATIONAL = 0,
    IMX_WATCH,
    IMX_ADVISORY,
    IMX_WARNING,
    IMX_WARNING_LEVELS
} imx_tsnumai_warning_t;
/*
 * Define how the CLI and status messages respond
 */
typedef enum {
    IMX_AT_VERBOSE_NONE = 0,
    IMX_AT_VERBOSE_STANDARD,
    IMX_AT_VERBOSE_STANDARD_STATUS
} imx_AT_versbose_mode_t;
/*
 * Define generic LED types.
 */
typedef enum {
    IMX_LED_ALL_OFF,
    IMX_LED_OFF,
    IMX_LED_ON,
    IMX_LED_OTHER,
    IMX_LED_INIT,
} imx_led_state_t;
/*
 * Define LED combinations
 */
typedef enum {
    IMX_LED_RED = 0,
    IMX_LED_GREEN,
    IMX_LED_BLUE,
    IMX_LED_RED_GREEN,
    IMX_LED_RED_BLUE,
    IMX_LED_GREEN_RED,
    IMX_LED_GREEN_BLUE,
    IMX_LED_BLUE_RED,
    IMX_LED_BLUE_GREEN,
    IMX_NO_LED_COMBINATIONS,
} imx_led_t;
/*
 * Define which interface iMatrix should use
 */
typedef enum {
    IMX_INTERFACE_ETHERNET,
    IMX_INTERFACE_WIFI
} imx_interface_t;

typedef uint32_t imx_status_t;
/*
 * Define variable length data pools
 */
typedef struct var_data_header {
    unsigned int pool_id    : 8;
    unsigned int reserved   : 24;
    void *next;
} imx_var_data_header_t;
/*
 * Define a variable length data structure
 */
typedef struct var_data_entry {
    imx_var_data_header_t header;
    uint16_t length;
    uint8_t *data;
} var_data_entry_t;

typedef struct var_data_block {
    var_data_entry_t *head;
    var_data_entry_t *tail;
} imx_var_data_block_t;
/*
 * Define generic 32 bit data or pointer to variable length record
 */
typedef union data_32 {
    uint32_t uint_32bit;
    int32_t int_32bit;
    float float_32bit;
    var_data_entry_t *var_data;
} imx_data_32_t;

typedef struct imx_var_data_config {
    uint16_t size;
    uint16_t no_entries;
} imx_var_data_config_t;

typedef struct imx_led_functions {
    void (*init_led)(uint16_t arg);
    imx_result_t (*set_led)(uint16_t arg, void *value);
} imx_led_functions_t;

typedef struct {
    char product_name[ IMX_PRODUCT_NAME_LENGTH + 1 ];
    char device_name[ IMX_DEVICE_NAME_LENGTH + 1 ];
    char imatrix_public_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char ota_public_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char manufacturing_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char imatrix_bind_uri[ IMX_IMATRIX_URI_LENGTH + 1 ];
    char default_ap_ssid[ IMX_SSID_LENGTH + 1];
    char default_ap_wpa[ IMX_WPA2PSK_LENGTH + 1 ];
    char default_st_ssid[ IMX_SSID_LENGTH + 1 ];
    char default_st_wpa[ IMX_WPA2PSK_LENGTH + 1 ];
    uint16_t default_ap_channel;
    uint16_t default_ap_eap_mode;
    uint16_t default_st_eap_mode;
    uint32_t default_ap_security_mode;
    uint32_t default_st_security_mode;
    uint16_t no_sensors;
    uint16_t no_controls;
    uint16_t host_major_version;
    uint16_t host_minor_version;
    uint16_t host_build_version;
    uint32_t product_capabilities;
    uint32_t product_id;
    uint32_t manufactuer_id;
    uint32_t building_id;
    uint32_t level_id;
    uint32_t indoor_x;
    uint32_t indoor_y;
    uint32_t sflash_size;
    float longitude;
    float latitude;
    float elevation;
    /*
     * Following items determine memory model setup
     */
    uint16_t history_size;
    uint16_t no_variable_length_pools;
    imx_var_data_config_t var_data_config[ IMX_MAX_VAR_LENGTH_POOLS ];
    /*
     * Following items are flags
     */
    unsigned int print_debugs           : 1;                // Do we print the set debug messages.
    unsigned int start_in_station_mode  : 1;                // Start Client with default Station Settings - Use for Development only
    unsigned int at_command_mode        : 1;                // What type of command interface is used
    unsigned int log_wifi_AP            : 1;
    unsigned int log_wifi_rssi          : 1;
    unsigned int log_wifi_rfnoise       : 1;
    imx_interface_t interface;
    imx_led_functions_t led_functions[ IMX_NO_LEDS ];   // Red, Green, Blue
    app_header_t lut[8];                                // The sflash lookup table.
} imx_imatrix_init_config_t;

typedef struct control_sensor_block {
    char name[ IMX_CONTROL_SENSOR_NAME_LENGTH ];
    uint32_t id;
    uint32_t sample_rate;
    uint32_t poll_rate;
    uint16_t sample_batch_size;
    uint16_t percent_change_to_send;            // Bits
    unsigned int enabled                : 1;    // 0    Is this entry used
    unsigned int read_only              : 1;    // 1    Read only Sensor (Internal - Can not change with AT command)
    unsigned int send_on_percent_change : 1;    // 2    Do you send update if this changes by a percentage
    unsigned int data_type              : 2;    // 3-4  Standard data types, Int Uint Float and Variable length
    unsigned int use_warning_level_low  : 3;    // 5-6  What warning levels for low levels do we notify on
    unsigned int use_warning_level_high : 3;    // 7-8  What warning levels for high levels do we notify on
    unsigned int set_default            : 1;    // 9    Does the system set the default value
    unsigned int send_imatrix           : 1;    // 10   Does the system send this entry to iMatrix
    unsigned int reserved               : 21;   // 11-31
    imx_data_32_t default_value;
    imx_data_32_t warning_level_low[ WARNING_LEVELS ];
    imx_data_32_t warning_level_high[ WARNING_LEVELS ];
} imx_control_sensor_block_t;

typedef struct functions {
    void (*load_config_defaults)(uint16_t arg);
    void (*init)(uint16_t arg);
    imx_result_t (*update)(uint16_t arg, void *value );
    uint16_t arg;
} imx_functions_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#endif /* COMMON_H_ */
