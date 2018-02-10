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
 * If no EULA applies, Sierra hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Sierra's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Sierra.
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
/** @file
 *
 *	config.c
 *	
 *	Read / Write the configuration
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../device_app_dct.h"
#include "../cli/interface.h"
#include "../cs_ctrl/common_config.h"
#include "../imatrix_upload/imatrix_upload.h"
#include "cert_util.h"
#include "icb_def.h"
#include "config.h"

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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern imx_imatrix_init_config_t imx_imatrix_init_config;
#include "factory_def.c"
/******************************************************
 *               Function Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
  * @brief  Load configuration from DCT - If invalid load factory defaults and save
  * @param  None
  * @retval : None
  */
wiced_result_t imatrix_load_config(bool override_config)
{
    platform_dct_mfg_info_t* dct_mfg_info = NULL;
    wiced_result_t result;
    uint16_t i;

    update_cert_pointers();// Make sure cert pointers are updated regardless of any errors accessing the DCT.
#ifdef USE_STM32
    result = wiced_dct_read_with_copy( &device_config, DCT_APP_SECTION, OFFSETOF( device_app_dct_t, config ), sizeof( IOT_Device_Config_t ) );
    if ( result != WICED_SUCCESS )
        return result;
#endif
    if ( ( device_config.valid_config == IMX_MAGIC_CONFIG) && ( override_config == false)  ){
        imx_printf( "Restored configuration from DCT" );
        if( ( device_config.host_major_version != imx_imatrix_init_config.host_major_version ) ||
            ( device_config.host_minor_version != imx_imatrix_init_config.host_minor_version ) ||
            ( device_config.host_build_version != imx_imatrix_init_config.host_build_version ) ) {
            /*
             * Update version number
             */
            device_config.host_major_version = imx_imatrix_init_config.host_major_version;
            device_config.host_minor_version = imx_imatrix_init_config.host_minor_version;
            device_config.host_build_version = imx_imatrix_init_config.host_build_version;
            imx_printf( "New HOST software version detected: " );
            imx_printf( IMX_VERSION_FORMAT, device_config.host_major_version, device_config.host_minor_version, device_config.host_build_version );
            imx_printf( "\r\n" );
            return imatrix_save_config();
        }
        imx_printf( "\r\n" );
        return WICED_SUCCESS;
    }

    // Replace invalid DCT with factory defaults and user defined values

    imx_printf( "*** Reseting to Factory Defaults ***\r\n" );
    /*
     * Start with know values and then update based on Host configuration
     *
     * Get SN from Manufacturing section of DCT
     */
    memcpy( &device_config, &factory_default_config, sizeof( IOT_Device_Config_t ) );
#ifdef USE_STM32
    result = wiced_dct_read_lock( (void**) &dct_mfg_info, WICED_FALSE, DCT_MFG_INFO_SECTION, 0, sizeof( *dct_mfg_info ) );
    if ( result != WICED_SUCCESS )
        return result;
#endif
    strncpy( device_config.device_serial_number, dct_mfg_info->serial_number, IMX_DEVICE_SERIAL_NUMBER_LENGTH );
    wiced_dct_read_unlock( dct_mfg_info, WICED_FALSE );

    /*
     * Set up system based on settings in user provided init structure
     */
    device_config.product_id = imx_imatrix_init_config.product_id;
    device_config.manufactuer_id = imx_imatrix_init_config.manufactuer_id;
    /*
     * Load the iMatrix URL and other configuration items that define this Thing
     */
    strncpy( device_config.imatrix_public_url, imx_imatrix_init_config.imatrix_public_url, IMX_IMATRIX_URL_LENGTH );
    strncpy( device_config.product_name, imx_imatrix_init_config.product_name, IMX_PRODUCT_NAME_LENGTH );
    /*
     * Default Device name as product name
     */
    strncpy( device_config.device_name, imx_imatrix_init_config.device_name, IMX_DEVICE_NAME_LENGTH );
    strncpy( device_config.ota_public_url, imx_imatrix_init_config.ota_public_url, IMX_IMATRIX_URL_LENGTH );
    strncpy( device_config.manufacturing_url, imx_imatrix_init_config.manufacturing_url, IMX_IMATRIX_URL_LENGTH );
    strncpy( device_config.imatrix_bind_uri, imx_imatrix_init_config.imatrix_bind_uri, IMX_IMATRIX_URI_LENGTH );
    strncpy( device_config.default_ap_ssid, imx_imatrix_init_config.default_ap_ssid, IMX_SSID_LENGTH );
    strncpy( device_config.default_ap_wpa, imx_imatrix_init_config.default_ap_wpa, IMX_WPA2PSK_LENGTH );
    strncpy( device_config.default_st_ssid, imx_imatrix_init_config.default_st_ssid, IMX_SSID_LENGTH );
    strncpy( device_config.default_st_wpa, imx_imatrix_init_config.default_st_wpa, IMX_WPA2PSK_LENGTH );
    strncpy( device_config.ap_ssid, imx_imatrix_init_config.default_ap_ssid, IMX_SSID_LENGTH );
    strncpy( device_config.ap_wpa, imx_imatrix_init_config.default_ap_wpa, IMX_WPA2PSK_LENGTH );
    strncpy( device_config.st_ssid, imx_imatrix_init_config.default_st_ssid, IMX_SSID_LENGTH );
    strncpy( device_config.st_wpa, imx_imatrix_init_config.default_st_wpa, IMX_WPA2PSK_LENGTH );
    device_config.default_ap_eap_mode = imx_imatrix_init_config.default_ap_eap_mode;
    device_config.default_st_eap_mode = imx_imatrix_init_config.default_st_eap_mode;
    device_config.default_ap_security_mode = imx_imatrix_init_config.default_ap_security_mode;
    device_config.default_st_security_mode = imx_imatrix_init_config.default_st_security_mode;
    device_config.ap_eap_mode = imx_imatrix_init_config.default_ap_eap_mode;
    device_config.st_eap_mode = imx_imatrix_init_config.default_st_eap_mode;
    device_config.ap_security_mode = imx_imatrix_init_config.default_ap_security_mode;
    device_config.st_security_mode = imx_imatrix_init_config.default_st_security_mode;
    device_config.default_ap_channel = imx_imatrix_init_config.default_ap_channel;
    device_config.ap_channel = device_config.default_ap_channel;
    if( imx_imatrix_init_config.start_in_station_mode == true ) {
        /*
         * App is forcing to use default settings Wi Fi to Station mode using presets
         */
        device_config.AP_setup_mode = false;
    }
    device_config.no_sensors = imx_imatrix_init_config.no_sensors;
    device_config.no_controls = imx_imatrix_init_config.no_controls;
    device_config.host_major_version = imx_imatrix_init_config.host_major_version;
    device_config.host_minor_version = imx_imatrix_init_config.host_minor_version;
    device_config.host_build_version = imx_imatrix_init_config.host_build_version;

    device_config.history_size = imx_imatrix_init_config.history_size;
    if( imx_imatrix_init_config.no_variable_length_pools > IMX_MAX_VAR_LENGTH_POOLS )
        device_config.no_variable_length_pools = IMX_MAX_VAR_LENGTH_POOLS;
    else
        device_config.no_variable_length_pools = imx_imatrix_init_config.no_variable_length_pools;
    for( i = 0; i < device_config.no_variable_length_pools; i++ ) {
        device_config.var_data_config[ i ].size = imx_imatrix_init_config.var_data_config[ i ].size;
        device_config.var_data_config[ i ].no_entries = imx_imatrix_init_config.var_data_config[ i ].no_entries;
    }
    device_config.building_id = imx_imatrix_init_config.building_id;
    device_config.product_capabilities = imx_imatrix_init_config.product_capabilities;
    device_config.level_id = imx_imatrix_init_config.level_id;
    device_config.indoor_x = imx_imatrix_init_config.indoor_x;
    device_config.indoor_y = imx_imatrix_init_config.indoor_y;
    device_config.longitude = imx_imatrix_init_config.longitude;
    device_config.latitude = imx_imatrix_init_config.latitude;
    device_config.elevation = imx_imatrix_init_config.elevation;
    device_config.sflash_size = imx_imatrix_init_config.sflash_size;
    device_config.at_command_mode = imx_imatrix_init_config.at_command_mode;
    device_config.log_wifi_AP = imx_imatrix_init_config.log_wifi_AP;
    device_config.log_wifi_rssi = imx_imatrix_init_config.log_wifi_rssi;
    device_config.log_wifi_rfnoise = imx_imatrix_init_config.log_wifi_rfnoise;

    /*
     * Do we have AT commands - Host processor will not expect extra status updates on serial interface
     */
    if( device_config.at_command_mode == true )  // We have AT controls - only show CLI output for direct requests and response to AT commands
        device_config.AT_verbose = IMX_AT_VERBOSE_STANDARD;
    else
        device_config.AT_verbose = IMX_AT_VERBOSE_STANDARD_STATUS;

#ifdef DEVELOPER_MODE
/*
 * Set up Developer SN, MAC and Passwords to save developer having to do this each time -
 * To get the data for this file - do a HTTP request to:
 * HTTP GET http://bind.imatrix.io/device?cpuid=0x0044001B3034510E36383536&productid=0x0B14ADFC
 * The format of the data returned is:
 * {"sn":"0404733148","mac":"00:06:8b:01:00:40","pw":"aD#49s27M1IXnTvE"}
 *
 * developer.inc file format should have following defines.
 *
 * #define DEVELOPER_SERIAL_NUMBER  "0404733148"
 * #define DEVELOPER_PASSWORD      "aD#49s27M1IXnTvE"
 * #define DEVELOPER_MAC_ADDRESS   "00:06:8b:01:00:40"
 *
 */
#include "../developer.inc"      // Located in Root of GitHub Repo - relocate
    char ch;
    int values[6], i;
    platform_dct_wifi_config_t* dct_wifi = NULL;
    platform_dct_wifi_config_t wifi;


    if( 6 == sscanf( DEVELOPER_MAC_ADDRESS, "%x:%x:%x:%x:%x:%x%c",
        &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], &ch ) ) {

        wiced_dct_read_with_copy( &dct_wifi, DCT_WIFI_CONFIG_SECTION, 0, sizeof( platform_dct_wifi_config_t ) );

        for( i = 0; i < 6; ++i )
            wifi.mac_address.octet[ i ] = (uint8_t) values[ i ];

        wiced_dct_write( &wifi, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );

        memset(  device_config.device_serial_number, 0x00, DEVICE_SERIAL_NUMBER_LENGTH );
        memset(  device_config.password, 0x00, PASSWORD_LENGTH );
        strncpy( device_config.device_serial_number, DEVELOPER_SERIAL_NUMBER, DEVICE_SERIAL_NUMBER_LENGTH );
        strncpy( device_config.password, DEVELOPER_PASSWORD, PASSWORD_LENGTH );

        imx_printf( "%c*** Developer Mode Active: Thing set to Serial No: %s, Password: %s, MAC: ", 0x07, device_config.device_serial_number, device_config.password );
        print_mac_address( (wiced_mac_t*) &wifi.mac_address );
        imx_printf( " ***\r\n" );

    }
#endif
    imx_printf( "User Configuration entries loaded\r\n" );

    cs_reset_defaults();

    return imatrix_save_config();
}

/**
  * Save the device configuration in the device_config variable into the DCT.
  *
  * This function assumes that the device configuration has already been loaded into the "device_config" variable.
  * Any error code returned by "wiced_dct_write()" is returned.
  *
  * written by Eric Thelin 30 September 2017
  */
wiced_result_t imatrix_save_config(void)
{
#ifdef USE_CYW943907
    return WICED_SUCCESS;
#endif
    return wiced_dct_write( &device_config, DCT_APP_SECTION, OFFSETOF( device_app_dct_t, config ), sizeof( IOT_Device_Config_t ) );
}

/**
  * @brief	print saved configuration
  * @param  None
  * @retval : None
  */

void imatrix_print_config( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

    cli_print( "Running WICED: %s, Name: %s, Manufacturing ID: 0x%08lx - ", WICED_VERSION, device_config.product_name, device_config.manufactuer_id, device_config.manufactuer_id );
	cli_print( "Active Configuration: - Magic: 0x%08lx\r\n", device_config.valid_config );
	cli_print( "Product Name: %s, Device Name: %s - ", device_config.product_name, device_config.device_name  );
	cli_print( "Serial Number: %08lX%08lX%08lX - iMatrix assigned: [%s]\r\n", device_config.sn.serial1, device_config.sn.serial2, device_config.sn.serial3, device_config.device_serial_number );
	cli_print( "Last NTP Updated time: %lu, Reboot Counter: %lu, Valid Config: 0x%08x\r\n", (uint32_t) device_config.last_ntp_updated_time, device_config.reboots, device_config.valid_config );
	cli_print( "Longitude %6.06f, Latitude: %6.06f, Time Offset from UTC: %2.2f\r\n", device_config.longitude, device_config.latitude, (float) device_config.local_seconds_offset_from_utc / ( 60 * 60 ) );
	cli_print( "Building ID: %lu, Level ID: %lu, Indoor Thing: %s, X: %lu, Y: %lu\r\n", device_config.building_id, device_config.level_id, device_config.indoor_device == true ? "True" : "False",
	        device_config.indoor_x, device_config.indoor_y );
    cli_print( "Default AP SSID: %s, Passphrase: ->%s<-, Security Mode: 0x%0x8l\r\n", device_config.default_ap_ssid, device_config.default_ap_wpa, device_config.default_ap_security_mode );
    cli_print( "Default ST SSID: %s, Passphrase: ->%s<-, Security Mode: 0x%0x8l\r\n", device_config.default_st_ssid, device_config.default_st_wpa, device_config.default_st_security_mode );
    cli_print( "Access Point Stored SSID: %s, Channel: %u, Passphrase: ->%s<-, Security Mode: 0x%0x8l\r\n", device_config.ap_ssid, device_config.ap_channel, device_config.ap_wpa, device_config.ap_security_mode );
    cli_print( "Station Stored SSID: %s, Passphrase: ->%s<-, Security Mode: 0x%0x8l\r\n", device_config.st_ssid, device_config.st_wpa, device_config.st_security_mode );
    cli_print( "Current Operating Mode: " );
    if( device_config.AP_setup_mode == true )
        cli_print( "Wi Fi Access Point: on Channel: %u", device_config.ap_channel );
    else
        cli_print( "Wi Fi Station" );

    cli_print( "\r\niMatrix URL: %s, iMatrix batch check time: %lumS, History Size: %u\r\n", device_config.imatrix_public_url, device_config.imatrix_batch_check_time, device_config.history_size );
	cli_print( "Manufacturing URL: %s, Bind URI: %s, OTA URL: %s\r\n", device_config.manufacturing_url, device_config.imatrix_bind_uri, device_config.ota_public_url );
	cli_print( "AT Variable Entry Timeout: %u mS, AT Verbose mode: %u\r\n", device_config.AT_variable_data_timeout, device_config.AT_verbose );
	cli_print( "Controls Configuration:\r\n" );
	print_common_config( IMX_CONTROLS, &device_config.ccb[ 0 ] );
	cli_print( "Sensors Configuration:\r\n" );
	print_common_config( IMX_SENSORS, &device_config.scb[ 0 ] );
}
/**
  * @brief	print saved configuration
  * @param  None
  * @retval : None
  */

wiced_result_t imatrix_print_saved_config( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	// The address in the DCT becomes invalid as soon as anything is written to the DCT
	IOT_Device_Config_t *temp_app_dct_config = (IOT_Device_Config_t*) ( (uint32_t)wiced_dct_get_current_address( DCT_APP_SECTION ) + OFFSETOF( device_app_dct_t, config ) );
	if ( temp_app_dct_config == GET_CURRENT_ADDRESS_FAILED ) {
	    cli_print( "DCT access error while attempting to print the saved device configuration.\r\n");
	    return WICED_ERROR;
	}

	cli_print( "DCT Configuration,saved @: 0x%08lx, Magic: 0x%08lx\r\n", (uint32_t) temp_app_dct_config, device_config.valid_config );
	cli_print( "Product Name: %s - Device Name: %s, ", temp_app_dct_config->product_name, temp_app_dct_config->device_name );
	cli_print( "Serial Number: %08lX%08lX%08lX", temp_app_dct_config->sn.serial1, temp_app_dct_config->sn.serial2, temp_app_dct_config->sn.serial3 );
	cli_print( " - iMatrix assigned: %s\r\n", temp_app_dct_config->device_serial_number );
	cli_print( "Last NTP Updated time: %lu, Reboot Counter: %lu, Valid Config: 0x%08x\r\n", (uint32_t) temp_app_dct_config->last_ntp_updated_time, temp_app_dct_config->reboots, temp_app_dct_config->valid_config );
	cli_print( "Longitude %6.06f, Latitude: %6.06f, Time Offset from UTC: %2.2f\r\n", temp_app_dct_config->longitude, temp_app_dct_config->latitude, (float) temp_app_dct_config->local_seconds_offset_from_utc / ( 60 * 60 ) );

    return WICED_SUCCESS;
}
