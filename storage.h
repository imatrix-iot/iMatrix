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

/** @file storage.h
 *
 *  Created on: October 8, 2017
 *      Author: greg.phillips
 *
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <wiced.h>

#include "common.h"
/*
 *	Defines for iMatrix Storage
 *
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define IMX_API_VERSION                     "1.0"
#define IMX_API_VERSION_LENGTH              6
#define IMX_IMATRIX_UNUSED                  0xFFFF
#define IMATRIX_SUCCESS                     0x00
#define IMATRIX_UNKNOWN_ARDUINO_CONTROL     0x001
#define IMATRIX_UNKNOWN_ARDUINO_SENSOR      0x002
#define IMATRIX_MUST_SUPPLY_DEFAULTS        0x100
#define IMATRIX_MAXIMUM_CONTROLS_EXCEEDED   0x200
#define IMATRIX_MUST_SUPPLY_CONTROL         0x201
#define IMATRIX_MAXIMUM_SENSORS_EXCEEDED    0x300
#define IMATRIX_MUST_SUPPLY_SAMPLE          0x301

#define IMX_AT_VAR_DATA_TIMEOUT             ( 10000 )   // 10 Seconds total for packet

#define IMX_MAX_CONNECTION_RETRY_COUNT      2

// For the ISM43362 we only have space for a reduced number of sensors and controls.
#ifdef _STM32F215RGT6_
#define IMX_MAX_NO_CONTROLS                 10
#define IMX_MAX_NO_SENSORS                  16
#else
#define IMX_MAX_NO_CONTROLS                 16
#define IMX_MAX_NO_SENSORS                  32
#endif

/*
 * Common characters and strings
 */
#define CHR_BELL                            0x07
#define CHR_BS                              0x08
#define CHR_TAB                             0x09
#define CHR_LF                              0x0a
#define CHR_CR                              0x0d
#define CRLF                                "\r\n"

#define CERT_BEGIN                          "-----BEGIN CERTIFICATE-----"
#define CERT_END                            "-----END CERTIFICATE-----"
#define KEY_BEGIN                           "-----BEGIN RSA PRIVATE KEY-----"
#define KEY_END                             "-----END RSA PRIVATE KEY-----"
/*
 *  Define the number of supported controls and sensors in the system
 *
 */

#define USE_WARNING_LEVEL_1                 ( 0x01 )    // Bit Masks
#define USE_WARNING_LEVEL_2                 ( 0x02 )
#define USE_WARNING_LEVEL_3                 ( 0x04 )


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
/*
 * Known Variable Length Data formats that we can print
 */
typedef enum var_data_types {
    VR_DATA_STRING,
    VR_DATA_MAC_ADDRESS,
    VR_DATA_BINARY,
    NO_VR_DATA_TYPES
} var_data_types_t;

typedef enum comm_mode {
    COMM_UDP,
    COMM_UDP_DTLS,
    COMM_TCP,
    COMM_TCP_TLS,
} comm_mode_t;

typedef enum security_mode {
    IMX_SECURITY_WEP,
    IMX_SECURITY_WPA2,
    IMX_SECURITY_8021X_EAP_TLS,
    IMX_SECURITY_8021X_PEAP,
    IMX_SECURITY_MAX_TYPES
} security_mode_t;
/*
 * iMatrix Block formats
 */
typedef enum imx_block {
    IMX_BLOCK_CONTROL = 0,
    IMX_BLOCK_SENSOR,
    IMX_BLOCK_MFG_UPDATE,
    IMX_BLOCK_RF_SCAN,
    IMX_BLOCK_GPS_COORDINATES,
    IMX_BLOCK_INDOOR_COORDINATES,
    IMX_BLOCK_IP_ADDRESS,
    IMX_BLOCK_EVENT_CONTROL,
    IMX_BLOCK_EVENT_SENSOR,
    IMX_NO_BLOCK_TYPES
} imx_block_t;

/*
 * Define well know system controls/sensors
 */
typedef enum  internal_predefined_sensors {
    IMX_INTERNAL_SENSOR_RF_SCAN = 0,
    IMX_INTERNAL_SENSOR_GPS_LATITUDE,
    IMX_INTERNAL_SENSOR_GPS_LONGITUDE,
    IMX_INTERNAL_SENSOR_GPS_ELEVATION,
    IMX_INTERNAL_SENSOR_INDOOR_BUILDING_ID,
    IMX_INTERNAL_SENSOR_INDOOR_LEVEL_ID,
    IMX_INTERNAL_SENSOR_INDOOR_X,
    IMX_INTERNAL_SENSOR_INDOOR_Y,
    IMX_INTERNAL_SENSOR_PRIVATE_IP_ADDRESS,
    IMX_INTERNAL_SENSOR_GATEWAY_IP_ADDRESS,
    IMX_INTERNAL_SENSOR_PUBLIC_IP_ADDRESS,
    IMX_INTERNAL_SENSOR_RF_CHANNEL,
    IMX_INTERNAL_SENSOR_RF_RSSI,
    IMX_INTERNAL_SENSOR_RF_NOISE,
    IMX_INTERNAL_SENSOR_RF_SSID,
    IMX_INTERNAL_SENSOR_RF_SECURITY,
    IMX_INTERNAL_SENSOR_THING_EVENT,
    IMX_NO_INTERNAL_SENSORS
} internal_predefined_sensors_t;

typedef enum internal_events {
    IMX_EVENT_NONE = 0,
    IMX_EVENT_REGISTRATION,
    IMX_NO_EVENTS,
} internal_events_t;

typedef enum imatrix_version {
    IMATRIX_VERSION_0 = 0,
    IMATRIX_VERSION_1 = 1,  // Current
    IMATRIX_VERSION_2 = 2,
    IMATRIX_VERSION_3 = 3,
    IMATRIX_VERSION_4 = 4,
    IMATRIX_VERSION_5 = 5,
    IMATRIX_VERSION_6 = 6,
    IMATRIX_VERSION_7 = 7,
} imatrix_version_t;

typedef struct serial_number {
    uint32_t serial1;
    uint32_t serial2;
    uint32_t serial3;
} serial_number_t;

typedef struct var_data {
    var_data_entry_t *entry;
    var_data_entry_t *next;
} var_data_t;


typedef struct control_sensor_data {
    unsigned int update_now             : 1;    // 0
    unsigned int warning                : 2;    // 1-2
    unsigned int last_warning           : 2;    // 2-3
    unsigned int send_batch             : 1;    // 4
    unsigned int error                  : 8;    // 5-12
    unsigned int last_error             : 8;    // 13-20
    unsigned int send_on_error          : 1;    // 21
    unsigned int active                 : 1;    // 1
    unsigned int valid                  : 1;    // 1    Has data be read yet
    unsigned int reserved               : 8;    // 24-31
    uint16_t no_samples;
    uint32_t errors;
    wiced_utc_time_ms_t last_sample_time;
    wiced_time_t last_poll_time;
    imx_data_32_t last_value;
    imx_data_32_t *data;
} control_sensor_data_t;

typedef union __attribute__((__packed__)) { // Bits
    struct {
        unsigned int block_type     : 4;    // 0-3
        unsigned int data_type      : 2;    // 4-5
        unsigned int warning        : 2;    // 6-7
        unsigned int no_samples     : 8;    // 8-15
        unsigned int sensor_error   : 8;    // 16-23
        unsigned int version        : 3;    // 24-26
        unsigned int reserved       : 5;    // 27-31
    } bits;
    uint32_t bit_data;
} bits_t;

typedef struct __attribute__((__packed__)) {        // Bytes
    bits_t bits;                                    // 0-3
    uint32_t id;                                    // 4-7
    wiced_utc_time_ms_t last_utc_ms_sample_time;    // 8-15
    uint32_t sample_rate;   // in mSec              // 16-19
} header_t ;

typedef struct __attribute__((__packed__)) {
    header_t header;
    imx_data_32_t data[];
} upload_data_t;

typedef struct IOT_Device_Config {
    char product_name[ IMX_PRODUCT_NAME_LENGTH + 1 ];
    char device_name[ IMX_DEVICE_NAME_LENGTH + 1 ];
    serial_number_t sn;
    char device_serial_number[ IMX_DEVICE_SERIAL_NUMBER_LENGTH + 1 ];
    char default_ap_ssid[ IMX_SSID_LENGTH + 1];
    char default_ap_wpa[ IMX_WPA2PSK_LENGTH + 1 ];
    char default_st_ssid[ IMX_SSID_LENGTH + 1 ];
    char default_st_wpa[ IMX_WPA2PSK_LENGTH + 1 ];
    char ap_ssid[ IMX_SSID_LENGTH + 1];
    char ap_wpa[ IMX_WPA2PSK_LENGTH + 1 ];
    char st_ssid[ IMX_SSID_LENGTH + 1 ];
    char st_wpa[ IMX_WPA2PSK_LENGTH + 1 ];
    char password[ IMX_PASSWORD_LENGTH + 1 ];
    char imatrix_public_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char ota_public_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char manufacturing_url[ IMX_IMATRIX_URL_LENGTH + 1 ];
    char imatrix_bind_uri[ IMX_IMATRIX_URI_LENGTH + 1 ];
    uint16_t reboots;// Space for a reboot counter if we want it for Known Good Configuration Logic that involves the bootloader.
    uint16_t no_sensors;
    uint16_t no_controls;
    uint16_t host_major_version;
    uint16_t host_minor_version;
    uint16_t host_build_version;
    uint16_t history_size;
    uint16_t no_variable_length_pools;
    uint16_t AT_variable_data_timeout;      // Duration for time for data to load a packet
    uint16_t default_ap_channel;
    uint16_t default_ap_eap_mode;
    uint16_t default_st_eap_mode;
    uint32_t default_ap_security_mode;
    uint32_t default_st_security_mode;
    uint16_t ap_channel;
    uint16_t ap_eap_mode;
    uint16_t st_eap_mode;
    uint32_t ap_security_mode;
    uint32_t st_security_mode;
    uint32_t imatrix_batch_check_time;
    uint32_t location_update_rate;
    uint32_t product_id;
    uint32_t manufactuer_id;
    uint32_t product_capabilities;
    uint32_t boot_count;
    uint32_t ota_fail_sflash_write;
    uint32_t ota_fail_sflash_crc;
    uint32_t ota_fail_communications_link;
    uint32_t valid_config;
    uint32_t building_id, floor_id, room_id, group_id, indoor_x, indoor_y, indoor_z;
    int32_t local_seconds_offset_from_utc;
    uint32_t log_messages;
    float longitude, latitude, elevation;
    uint32_t sflash_size;
    wiced_security_t ap_security;
    wiced_utc_time_ms_t last_ntp_updated_time;
    imx_control_sensor_block_t ccb[ IMX_MAX_NO_CONTROLS ];
    imx_control_sensor_block_t scb[ IMX_MAX_NO_SENSORS ];
    imx_var_data_config_t var_data_config[ IMX_MAX_VAR_LENGTH_POOLS ];
    unsigned int print_debugs               : 1;    // 0
    unsigned int log_wifi_AP                : 1;    // 1 - Log Wi Fi Events and levels
    unsigned int log_wifi_rssi              : 1;    // 2
    unsigned int log_wifi_rfnoise           : 1;    // 3
    unsigned int send_logs_to_imatrix       : 1;    // 4 Send log messages to iMatrix
    unsigned int at_command_mode            : 1;    // 5 Determines CLI output styles
    unsigned int application_loaded         : 1;    // 6 Has the application already loaded
    unsigned int api_loaded                 : 1;    // 7 Has API loaded values - use to overide defaults
    unsigned int imatrix_enabled            : 1;    // 8
    unsigned int cli_enabled                : 1;    // 9
    unsigned int telnet_enabled             : 1;    // 10
    unsigned int ssh_enabled                : 1;    // 11
    unsigned int username_password_enabled  : 1;    // 12
    unsigned int comm_mode                  : 4;    // 13-16 Used for iMatrix Communication options
    unsigned int mobile_thing               : 1;    // 17 - Send regular location updates
    unsigned int indoor_thing               : 1;    // 18 - Do we send both GPS or indoor locations?
    unsigned int enable_imatrix             : 1;    // 19
    unsigned int use_rssi                   : 1;    // 20
    unsigned int use_rfnoise                : 1;    // 21
    unsigned int use_wifi_channel           : 1;    // 22
    unsigned int use_temperatue             : 1;    // 23
    unsigned int use_red_led                : 1;    // 24
    unsigned int use_green_led              : 1;    // 25
    unsigned int send_now_on_warning_level  : 2;    // 26-27
    unsigned int provisioned                : 1;    // 28
    unsigned int do_SFLASH_load             : 1;    // 29 - On boot load the SFLASH from a known source
    unsigned int AP_setup_mode              : 1;    // 30 - Operate as an Access Point to enable other device to be programmed with AP settings to use
    unsigned int connected_to_imatrix       : 1;    // 31 - Set to true once a device has successfully connected to iMatrix Server
    unsigned int log_to_imatrix             : 1;    // 32
    unsigned int AT_echo                    : 1;    // 33 - Echo characters 0 - Disable / 1 - Enable
    unsigned int AT_verbose                 : 2;    // 34 - Verbose mode - 0 - No response / 1 Standard Response / 2 - Standard + ISMART Status messages / 3 - Undefined
    unsigned int reserved                   : 30;   // 33 - 63
} IOT_Device_Config_t;



/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
imx_status_t init_storage(void);
void *imx_allocate_storage( uint16_t size );
#endif /* STORAGE_H_ */
