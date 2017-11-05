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

/*
 *  Define the number of sensors and controls in the system
 *
 */
#define IMX_MAX_NO_SENSORS              ( 5 + 32 )      // 5 Integrated Wi Fi Channel, BSSID, RSSI, Noise, On Board Temp - 32 User Defined
#define IMX_MAX_NO_CONTROLS             ( 0 + 16 )      // 16 User Defined
/*
 *  Define the number of Smart Arduino controls and sensors in the system
 *
 */
#define IMX_MAX_ARDUINO_CONTROLS        ( 8 )
#define IMX_MAX_ARDUINO_SENSORS         ( 8 )

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
/*
 * System status: Operating Mode Setup / Standalone mode - or normal online mode
 */
bool imx_get_wifi_mode(void);
bool imx_network_connected(void);
char *imx_get_device_serial_number(void);
/*
 * Apollo Configuration management
 */
wiced_result_t imx_get_config_current_address( void *config_address );
wiced_result_t imx_save_config( void *config, uint16_t config_size );
/*
 * Console I/O
 */
bool imx_get_ch( char *ch );
void imx_printf( char *format, ... );
/*
 * Set & Get Control / Sensor data
 */
imx_status_t imx_set_sensor( uint16_t sensor_entry, void *value );
imx_status_t imx_get_sensor( uint16_t sensor_entry, void *value );
imx_status_t imx_set_control( uint16_t sensor_entry, void *value );
imx_status_t imx_get_control( uint16_t control_entry, void *value );
/*
 * Standard Control/Sensor Processing functions
 */
void load_config_defaults_generic_scb( uint16_t arg );
void load_config_defaults_generic_ccb( uint16_t arg );
/*
 * LED Control functions
 */
bool imx_set_led( imx_led_t led, imx_led_state_t mode );
bool imx_get_led_state( imx_led_t led );
/*
 * Additional support Platform code - ISMART-43340
 */
void imx_init_led_red_ismart( void );
void imx_init_led_green_ismart( void );
void imx_init_led_blue_ismart( void );
void imx_update_led_red_status_ismart( bool state );
void imx_update_led_green_status_ismart( bool state );
void imx_update_led_blue_status_ismart( bool state );
void imx_init_temp(uint16_t arg);
uint16_t imx_sample_temp(uint16_t arg, void *value );
/*
 * General Wi Fi Status routines
 */
uint16_t imx_get_wifi_channel_scb(void);
uint16_t imx_get_wifi_rssi_scb(void);
uint16_t imx_get_wifi_bssid_scb(void);
uint16_t imx_get_wifi_rf_noise_scb(void);
uint16_t imx_sample_rssi(uint16_t arg, void *value );
uint16_t imx_sample_rfnoise(uint16_t arg, void *value );
uint16_t imx_sample_wifi_channel(uint16_t arg, void *value );
/*
 * UTC Time Update
 */
uint16_t imx_get_utc(uint16_t arg, void *value );
/*
 * CoAP Processing defines
 */
#include "coap/coap.h"
/*
 * Include JSON library
 */
#include "json/mjson.h"
#endif /* IMX_IMATRIX_H_ */
