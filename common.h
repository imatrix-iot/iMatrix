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

#define IMX_SUCCESS                     0x00
#define IMX_UNKNOWN_ARDUINO_CONTROL     0x001
#define IMX_UNKNOWN_ARDUINO_SENSOR      0x002
#define IMX_MUST_SUPPLY_DEFAULTS        0x100
#define IMX_MAXIMUM_CONTROLS_EXCEEDED   0x200
#define IMX_MUST_SUPPLY_CONTROL         0x201
#define IMX_MAXIMUM_SENSORS_EXCEEDED    0x300
#define IMX_MUST_SUPPLY_SAMPLE          0x301
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
    IMX_NO_ERROR = 0,
    IMX_GENERAL_FAILURE,
    IMX_INIT_ERROR,
    IMX_I2C_ERROR,
    IMX_SPI_ERROR,
    IMX_INVALID_READING,
    IMX_FAIL_CRC,
    IMX_BAD_ARDUINO
} imx_errors_t;

/*
 * Define data types for Controls
 */
typedef enum {
    IMX_DO_UINT32 = 0,
    IMX_DO_INT32,
    IMX_AO_FLOAT,
    IMX_DO_VARIABLE_LENGTH,
} imx_do_types_t;
/*
 * Define data types for Sensors
 */
typedef enum {
    IMX_DI_UINT32 = 0,
    IMX_DI_INT32,
    IMX_AI_FLOAT,
    IMX_DI_VARIABLE_LENGTH,
} imx_di_types_t;

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
} imx_led_state_t;

typedef enum {
    IMX_LED_RED = 0,
    IMX_LED_GREEN,
    IMS_LED_BLUE,
    IMX_LED_RED_GREEN,
    IMX_LED_RED_BLUE,
    IMX_LED_GREEN_BLUE,
} imx_led_t;

typedef uint32_t imx_status_t;

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
    uint16_t no_at_controls;
    uint16_t at_control_start;
    uint16_t no_at_sensors;
    uint16_t at_sensor_start;
    uint16_t ap_eap_mode;
    uint16_t st_eap_mode;
    uint32_t ap_security_mode;
    uint32_t st_security_mode;
    uint32_t product_capabilities;
    uint32_t product_id;
    uint32_t organization_id;
    uint32_t building_id;
    uint32_t level_id;
    uint32_t indoor_x;
    uint32_t indoor_y;
    float longitude;
    float latitude;
    float elevation;
    bool (*set_led)( imx_led_t led, uint16_t state );
} imx_imatrix_init_config_t;
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#endif /* COMMON_H_ */
