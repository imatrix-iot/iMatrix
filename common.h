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
#define IMX_DEVICE_NAME_LENGTH          ( IMX_PRODUCT_NAME_LENGTH )
#define IMX_DEVICE_SERIAL_NUMBER_LENGTH ( 10 )
#define IMX_SAMPLE_LENGTH               ( 4 )
#define IMX_MAX_NO_BLE_DEVICES          ( 20 )
#define IMX_MAC_ADDRESS_LENGTH          ( 6 )
#define IMX_NO_RF_SCAN_RECORDS          ( 20 )
#define IMX_ORGANIZATION_ID_LENGTH      ( 10 )
#define IMX_PRODUCT_ID_LENGTH           ( 10 )

/*
 * Wi-Fi Credentials length
 */
#define IMX_WPA2PSK_LENGTH              ( 63 )
#define IMX_SSID_LENGTH                 ( 32 )
#define IMX_PASSWORD_LENGTH             ( 32 )
#define IMX_IMATRIX_URL_LENGTH          ( 64 )
#define IMX_IMATRIX_URI_LENGTH          ( 64 )
#define IMX_IMATRIX_SITE_LENGTH         ( 64 )

//#define IMX_USE_HOT_SPOT

#ifdef USE_HOT_SPOT
#define IMX_DEFAULT_ST_SSID             "2ROOS Mobile"
#define IMX_DEFAULT_ST_KEY              "happydog"
#define IMX_DEFAULT_ST_SECURITY         WICED_SECURITY_WPA2_AES_PSK
#else
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
#define IMX_DEFAULT_ST_SSID             "SierraTelecom"
#define IMX_DEFAULT_ST_KEY              "happydog"
#define IMX_DEFAULT_ST_SECURITY         WICED_SECURITY_WPA2_AES_PSK
#endif

#define IMX_DEFAULT_AP_SSID             "ISMART-ConnectKit"
#define IMX_DEFAULT_AP_KEY              ""
#define IMX_DEFAULT_AP_SECURITY         WICED_SECURITY_OPEN

#define IMX_SEC_IN_DAY                  ( 24UL * 60UL * 60UL )
/*
 * Location defaults - Zephyr Cove Office
 */
#define IMX_LONGITUDE_DEFAULT           -119.943016
#define IMX_LATITUDE_DEFAULT            38.986835
#define IMX_ELEVATION_DEFAULT           1925.15         // Elevation in Meters

#define WARNING_LEVELS                  ( 3 )
#define IMX_NO_LEDS                     ( 3 )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
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
 * Define Control and Sensor Errors
 */
typedef enum {
    IMX_SUCCESS = 0,
    IMX_INVALID_ENTRY,
    IMX_CONTROL_DISABLED,
    IMX_SENSOR_DISABLED,
    IMX_GENERAL_FAILURE,
    IMX_INIT_ERROR,
    IMX_I2C_ERROR,
    IMX_SPI_ERROR,
    IMX_INVALID_READING,
    IMX_FAIL_CRC,
    IMX_BAD_ARDUINO,
    IMX_UNKNOWN_ARDUINO_CONTROL,
    IMX_UNKNOWN_ARDUINO_SENSOR,
    IMX_MUST_SUPPLY_DEFAULTS,
    IMX_MAXIMUM_CONTROLS_EXCEEDED,
    IMX_MUST_SUPPLY_CONTROL,
    IMX_MAXIMUM_SENSORS_EXCEEDED,
    IMX_MUST_SUPPLY_SAMPLE,
} imx_errors_t;

/*
 * Define data types for Controls & Sensors
 */
typedef enum {
    IMX_UINT32 = 0,
    IMX_INT32,
    IMX_FLOAT,
    IMX_VARIABLE_LENGTH,
} imx_data_types_t;

typedef enum {
    IMX_AT_VERBOSE_NONE = 0,
    IMX_AT_VERBOSE_STANDARD,
    IMX_AT_VERBOSE_STANDARD_STATUS
} imx_AT_versbose_mode_t;

typedef enum {
    IMX_LED_OFF = 0,
    IMX_LED_ON,
    IMX_LED_BLINK_1,
    IMX_LED_BLINK_2,
    IMX_LED_BLINK_3,
    IMX_LED_BLINK_4,
    IMX_LED_BLINK_5,
    IMX_LED_BLINK_6,
    IMX_LED_BLINK_7,
    IMX_LED_BLINK_8,
    IMX_LED_BLINK_9,
    IMX_LED_BLINK_10,
    IMX_LED_BLINK_MASK = 0x0FF,
    IMX_LED_FLASH_1 = 0x100,
    IMX_LED_FLASH_2 = 0x200,
    IMX_LED_FLASH_3 = 0x300,
    IMX_LED_FLASH_4 = 0x400,
    IMX_LED_FLASH_5 = 0x500,
    IMX_LED_FLASH_6 = 0x600,
    IMX_LED_FLASH_7 = 0x700,
    IMX_LED_FLASH_8 = 0x800,
    IMX_LED_FLASH_9 = 0x900,
    IMX_LED_FLASH_10 = 0xA00,
    IMX_LED_FLASH_MASK = 0xF00,
    IMX_LED_FLASH = 0x1000,  // Indicate this is a flash not a blink on / off - Blink rate then represents duty cycle in 1 second
} imx_led_state_t;

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
    IMX_LED_INIT = 0xFF,
} imx_led_t;

typedef uint32_t imx_status_t;

typedef struct var_data_header {
    unsigned int pool_id : 8;
    unsigned int reserved : 8;
    unsigned int length : 16;
    void *next;
} var_data_header_t;

typedef struct var_data_entry {
    var_data_header_t header;
    uint8_t data[];
} var_data_entry_t;

typedef struct var_data_block {
    var_data_entry_t *head;
    var_data_entry_t *tail;
} var_data_block_t;

typedef union data_32 {
    uint32_t uint_32bit;
    int32_t int_32bit;
    float float_32bit;
    var_data_entry_t *var_data;
} data_32_t;

typedef struct imx_led_functions {
    void (*init_led)(void);
    void (*set_led)( bool state );
} imx_led_functions_t;

typedef struct {
    char product_name[ IMX_PRODUCT_NAME_LENGTH + 1 ];
    char device_name[ IMX_DEVICE_NAME_LENGTH + 1 ];
    char imatrix_public_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char ota_public_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char manufacturing_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    uint16_t no_sensors;
    uint16_t no_controls;
    uint16_t no_arduino_sensors;
    uint16_t no_arduino_controls;
    uint16_t ap_eap_mode;
    uint16_t st_eap_mode;
    uint32_t ap_security_mode;
    uint32_t st_security_mode;
    uint32_t product_capabilities;
    uint32_t product_id;
    uint32_t manufactuer_id;
    uint32_t building_id;
    uint32_t level_id;
    uint32_t indoor_x;
    uint32_t indoor_y;
    float longitude;
    float latitude;
    float elevation;
    unsigned int at_command_mode    : 1;                // What type of command interface is used
    unsigned int log_wifi_AP        : 1;
    unsigned int log_wifi_rssi      : 1;
    unsigned int log_wifi_rfnoise   : 1;
    imx_led_functions_t led_functions[ IMX_NO_LEDS ];   // Red, Green, Blue
} imx_imatrix_init_config_t;

typedef struct control_sensor_block {
    char name[ IMX_CONTROL_SENSOR_NAME_LENGTH ];
    uint32_t id;
    uint32_t sample_rate;
    uint16_t sample_batch_size;
    uint16_t percent_change_to_send;            // Bits
    unsigned int enabled                : 1;    // 0    Is this entry used
    unsigned int read_only              : 1;    // 1    Read only Sensor (Internal - Can not change with AT command)
    unsigned int valid                  : 1;    // 2    Has data be read yet
    unsigned int send_on_percent_change : 1;    // 3
    unsigned int data_type              : 2;    // 4-5  Standard data types, Int Uint Float and Variable length
    unsigned int use_warning_level_low  : 3;    // 6-7
    unsigned int use_warning_level_high : 3;    // 8-9
    unsigned int reserved               : 17;   // 10-31
    data_32_t default_value;
    data_32_t warning_level_low[ WARNING_LEVELS ];
    data_32_t warning_level_high[ WARNING_LEVELS ];
} imx_control_sensor_block_t;

typedef struct functions {
    void (*load_config_defaults)(uint16_t arg);
    void (*init)(uint16_t arg);
    uint16_t (*update)(uint16_t arg, void *value );
    uint16_t arg;
} imx_functions_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#endif /* COMMON_H_ */
