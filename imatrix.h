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

/** @file imatirx.h
 *
 *  Created on: October 8, 2017
 *      Author: greg.phillips
 *
 */

#ifndef IMX_IMATRIX_H_
#define IMX_IMATRIX_H_

#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "common.h"
#include "platform_functions/onewire.h"
#include "platform_functions/ISMART.h"
/*
 *	Defines for iMatrix API
 *
 * This file contains add API defines and functions used by the iMatrix Library
 *
 * The following is a template for the initial settings of the iMatrix Config structure
 *
imx_imatrix_init_config_t imatrix_config = {
    .product_name = IMX_PRODUCT_NAME,
    .device_name = IMX_PRODUCT_NAME,
    .imatrix_public_url = IMX_IMATRIX_SITE,
    .ota_public_url = IMX_OTA_SITE,
    .no_sensors = IMX_NO_SENSORS,
    .no_controls = IMX_NO_CONTROLS,
    .no_arduino_sensors = IMX_NO_ARDUINO_SENSORS,
    .no_arduino_controls = IMX_NO_ARDUINO_CONTROLS,
    .ap_eap_mode = 0,
    .st_eap_mode = 0,
    .ap_security_mode = WICED_SECURITY_OPEN,
    .st_security_mode = IMX_DEFAULT_ST_SECURITY,
    .product_capabilities = ( IMX_WIFI_2_4GHZ | IMX_WIFI_5_2GHZ |IMX_WIFI_5_4GHZ | IMX_WIFI_5_8GHZ ),
    .product_id = IMX_PRODUCT_ID,
    .organization_id = IMX_ORGANIZATION_ID,
    .building_id = 0,
    .level_id = 0,
    .indoor_x = 0,
    .indoor_y = 0,
    .longitude = IMX_LONGITUDE_DEFAULT,
    .Latitude = IMX_LATITUDE_DEFAULT,
    .elevation = IMX_ELEVATION_DEFAULT
    bool (*set_led)( imx_led_t led, uint16_t state );
};
 *
 */
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/*
 * Start up and run functions
 */
imx_status_t imx_init( imx_imatrix_init_config_t *init_config, bool override_config, bool run_in_background );
imx_status_t imx_process(void);
imx_status_t imx_deinit(void);
void imx_boot_factory_reset(void);
/*
 * Watchdog functions
 */
void imx_init_watchdog(void);
void imx_kick_watchdog(void);
/*
 * System status: Operating Mode Setup / Standalone mode - or normal online mode
 */
imx_wifi_mode_t imx_get_wifi_mode(void);
bool imx_network_connected(void);
char *imx_get_device_serial_number(void);
/*
 * Host App Configuration management
 */
wiced_result_t imx_get_config_current_address( void **config_address );
wiced_result_t imx_save_config( void *config, uint16_t config_size );
void imx_set_imatrix_debug_flags( uint32_t debug_flags );
uint32_t imx_get_imatrix_debug_flags(void);

/*
 * CLI - Console I/O
 */
bool imx_get_ch( char *ch );
void imx_printf( char *format, ... );
int imx_log_printf( char *format, ... );
bool imx_verify_cmd(void);
void imx_set_cli_handler( bool (*cli_handler)( char *token ) );
/*
 * Set & Get Control / Sensor data
 */
imx_status_t imx_set_control_sensor( imx_peripheral_type_t type, uint16_t entry, void *value );
imx_status_t imx_get_control_sensor( imx_peripheral_type_t type, uint16_t entry, void *value );

/*
 * Standard Control/Sensor Processing functions
 */
void load_config_defaults_generic_scb( uint16_t arg );
void load_config_defaults_generic_ccb( uint16_t arg );
/*
 * LED Control functions
 */
bool imx_set_led( imx_led_t led, imx_led_state_t mode, uint16_t mode_details );
bool imx_get_led_state( imx_led_t led );
/*
 * Location functions
 */
float imx_get_longitude(void);
void imx_set_longitude(float longitude);
float imx_get_latitude(void);
void imx_set_latitude(float latitude);
float imx_get_elevation(void);
void imx_set_elevation(float elevation);
uint32_t imx_get_indoor_x(void);
void imx_set_indoor_x(uint32_t indoor_x);
uint32_t imx_get_indoor_y(void);
void imx_set_indoor_y(uint32_t indoor_y);
uint32_t imx_get_indoor_z(void);
void imx_set_indoor_z(uint32_t indoor_z);
/*
 * General Wi Fi Status routines
 */
void imx_init_wi_fi_bssid(uint16_t arg);
imx_result_t  imx_sample_wi_fi_bssid(uint16_t arg, void *value );
void imx_init_wi_fi_channel(uint16_t arg);
imx_result_t  imx_sample_wi_fi_channel(uint16_t arg, void *value );
uint16_t imx_get_host_s_w_version_scb(void);
uint16_t imx_get_wifi_channel_scb(void);
uint16_t imx_get_wifi_rssi_scb(void);
uint16_t imx_get_wifi_bssid_scb(void);
uint16_t imx_get_wifi_rf_noise_scb(void);
void imx_init_rssi(uint16_t arg);
imx_result_t imx_sample_rssi(uint16_t arg, void *value );
void imx_init_rf_noise(uint16_t arg);
imx_result_t imx_sample_rf_noise(uint16_t arg, void *value );
imx_result_t imx_sample_wifi_channel(uint16_t arg, void *value );
void imx_set_wifi_notification( void (*wifi_notification)( bool state ) );
uint32_t imx_get_wifi_success_count(void);
uint32_t imx_get_wifi_failed_count(void);
/*
 * Time & Watchdogs Update
 */
void imx_set_local_seconds_offset_from_utc( int32_t local_seconds_offset_from_utc );
int32_t imx_get_local_seconds_offset_from_utc(void);
wiced_utc_time_t imx_current_local_time(void);
uint16_t imx_ntp_succeeded_at_least_once(void);
uint16_t imx_day_of_week(wiced_utc_time_t seconds_since_1969);
bool imx_is_later(  wiced_time_t time1, wiced_time_t time2 );
imx_result_t imx_get_utc(uint16_t arg, void *value );
void imx_kick_watchdog(void);
/*
 * Memory management - used to take advantage of CCMSRAM space - will allocate from regular heap - but never freed
 */
void *imx_allocate_storage( uint16_t size );
void imx_add_var_free_pool( var_data_entry_t *var_data_ptr );
var_data_entry_t *imx_get_var_data( uint16_t length );
/*
 * IP & CoAP Processing defines
 */
#include "coap/coap.h"
bool imx_is_multicast_ip( wiced_ip_address_t *addr );
void imx_set_host_coap_interface( uint16_t no_coap_entries, CoAP_entry_t *host_coap_entries );
bool imx_supress_multicast_response( wiced_ip_address_t *addr, uint16_t response );
/*
 * Include JSON library
 */
#include "json/mjson.h"
#endif /* IMX_IMATRIX_H_ */
