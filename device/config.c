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
#include "../imatrix/imatrix.h"
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
extern iMatrix_Control_Block_t icb;
extern imx_imatrix_init_config_t imatrix_init_config;
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
wiced_result_t imatrix_load_config(void)
{
   wiced_result_t result;

    update_cert_pointers();// Make sure cert pointers are updated regardless of any errors accessing the DCT.

    result = wiced_dct_read_with_copy( &device_config, DCT_APP_SECTION, OFFSETOF( device_app_dct_t, config ), sizeof( IOT_Device_Config_t ) );
    if ( result != WICED_SUCCESS )
        return result;

    if ( device_config.valid_config == IMX_MAGIC_CONFIG) {
        print_status( "Restored configuration from DCT\r\n" );
        return WICED_SUCCESS;
    }

    // Replace invalid DCT with factory defaults and user defined values

    print_status( "Reseting to factory defaults\r\n" );
    memcpy( &device_config, &factory_default_config, sizeof( IOT_Device_Config_t ) );

    strncpy( device_config.product_name, imatrix_init_config.product_name, IMX_PRODUCT_NAME_LENGTH );
    /*
     * Default Device name as product name
     */
    strncpy( device_config.device_name, imatrix_init_config.product_name, IMX_DEVICE_NAME_LENGTH );

    device_config.product_id = imatrix_init_config.product_id;
    device_config.organization_id = imatrix_init_config.organization_id;
    /*
     * Load the iMatrix URL and other configuration items that define this Thing
     */
    strncpy( device_config.imatrix_public_url, imatrix_init_config.imatrix_public_url, IMX_IMATRIX_URL_LENGTH );
    device_config.no_sensors = imatrix_init_config.no_sensors;
    device_config.no_controls = imatrix_init_config.no_controls;
    device_config.no_arduino_sensors = imatrix_init_config.no_arduino_sensors;
    device_config.no_arduino_controls = imatrix_init_config.no_arduino_controls;
    device_config.no_at_controls = imatrix_init_config.no_at_controls;
    device_config.at_control_start = imatrix_init_config.at_control_start;
    device_config.no_at_sensors = imatrix_init_config.no_at_sensors;
    device_config.at_sensor_start = imatrix_init_config.at_sensor_start;
    device_config.ap_eap_mode = imatrix_init_config.ap_eap_mode;
    device_config.st_eap_mode = imatrix_init_config.st_eap_mode;
    device_config.ap_security_mode = imatrix_init_config.ap_security_mode;
    device_config.st_security_mode = imatrix_init_config.st_security_mode;
    device_config.building_id = imatrix_init_config.building_id;
    device_config.product_capabilities = imatrix_init_config.product_capabilities;
    device_config.level_id = imatrix_init_config.level_id;
    device_config.indoor_x = imatrix_init_config.indoor_x;
    device_config.indoor_y = imatrix_init_config.indoor_y;
    device_config.longitude = imatrix_init_config.longitude;
    device_config.lattitude = imatrix_init_config.lattitude;
    device_config.elevation = imatrix_init_config.elevation;

    /*
     * Do we have AT commands - Host processor will not expect extra status updates on serial interface
     */
    if( ( device_config.no_at_controls + device_config.no_at_sensors ) > 0 )  // We have AT controls - only show CLI output for direct requests and response to AT commands
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

        print_status( "%c*** Developer Mode Active: Thing set to Serial No: %s, Password: %s, MAC: ", 0x07, device_config.device_serial_number, device_config.password );
        print_mac_address( (wiced_mac_t*) &wifi.mac_address );
        print_status( " ***\r\n" );

    }
#endif

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

	cli_print( "Active Configuration: - Magic: 0x%08lx\r\n", device_config.valid_config );
	cli_print( "Product Name: %s, Device Name: %s - ", device_config.product_name, device_config.device_name  );
	cli_print( "Serial Number: %08lX%08lX%08lX - iMatrix assigned: %s\r\n", device_config.sn.serial1, device_config.sn.serial2, device_config.sn.serial3, device_config.device_serial_number );
	cli_print( "Last NTP Updated time: %lu, Reboot Counter: %lu, Valid Config: 0x%08x\r\n", (uint32_t) device_config.last_ntp_updated_time, device_config.reboots, device_config.valid_config );
	cli_print( "Longitude %6.06f, Latitude: %6.06f, Time Offset from UTC: %2.2f\r\n", device_config.longitude, device_config.lattitude, (float) device_config.local_seconds_offset_from_utc / ( 60 * 60 ) );
	cli_print( "Operating Mode: %s, SSID: %s, Passphrase: ->%s<-\r\n", device_config.AP_setup_mode ? "Provisioning" : "Normal Online", device_config.st_ssid, device_config.st_wpa );
	imatrix_print_config( 0 );

	cli_print( "\r\n" );
	imatrix_print_saved_config( 0 );

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
	cli_print( "Longitude %6.06f, Latitude: %6.06f, Time Offset from UTC: %2.2f\r\n", temp_app_dct_config->longitude, temp_app_dct_config->lattitude, (float) temp_app_dct_config->local_seconds_offset_from_utc / ( 60 * 60 ) );

    return WICED_SUCCESS;
}
