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

/*
 * print_dct.c
 *
 *  Created on: Nov 2, 2016
 *      Author: Eric Thelin
 */

#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"
#include "../storage.h"
#include "../device_app_dct.h"
#include "../networking/utility.h"
#include "interface.h"
#include "print_dct.h"

void print_dct( uint16_t arg )
{

	UNUSED_PARAMETER(arg);

	print_dct_header();
	print_security_dct();
	print_mfg_info_dct();
	print_wifi_config_dct();
	print_app_dct();
	print_network_config_dct();
}

wiced_result_t print_dct_header( void )
{
	platform_dct_header_t* dct_header = NULL;

    if ( wiced_dct_read_lock( (void**) &dct_header, WICED_FALSE, DCT_INTERNAL_SECTION, 0, sizeof( *dct_header ) ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    /* since we passed ptr_is_writable as WICED_FALSE, we are not allowed to write in to memory pointed by dct_security */

    /* Header Section */
    imx_cli_print( "DCT Header Section \r\n");
    imx_cli_print( "Full size: %l u, Used size: %lu.\r\n", dct_header->full_size, dct_header->used_size );
    imx_cli_print( "Write incomplete: %s, is current DCT: %s, App valid: %s MFG info programmed: %s.\r\n",
            ( dct_header->write_incomplete ? "True" : "False" ),
            ( dct_header->is_current_dct ? "True" : "False" ),
            ( dct_header->app_valid ? "True" : "False" ),
            ( dct_header->mfg_info_programmed ? "True" : "False" ) );

    imx_cli_print( "Magic number: %lu, Entry point: %lu, Boot valid: %s Load once: %s.\r\n", dct_header->magic_number, dct_header->boot_detail.entry_point,
            ( dct_header->boot_detail.load_details.valid ? "True" : "False" ),
            ( dct_header->boot_detail.load_details.load_once ? "True" : "False" ) );
    if ( dct_header->boot_detail.load_details.source.id == EXTERNAL_FIXED_LOCATION ) {
    	imx_cli_print( "DCT external source: 0x%lx, size %lu, ",
		     dct_header->boot_detail.load_details.source.detail.external_fixed.location,
		     dct_header->boot_detail.load_details.source.detail.external_fixed.size );
    }
    else if ( dct_header->boot_detail.load_details.source.id == INTERNAL ) {
    	imx_cli_print( "DCT internal source: 0x%lx, size %lu, ",
			dct_header->boot_detail.load_details.source.detail.internal_fixed.location,
			dct_header->boot_detail.load_details.source.detail.internal_fixed.size );
    }
    else if ( dct_header->boot_detail.load_details.source.id == NONE ) {
    	imx_cli_print( "No DCT boot source.\r\n" );
    }
    else {
    	imx_cli_print( "Invalid boot source.\r\n");
    }
    if ( dct_header->boot_detail.load_details.destination.id == INTERNAL ) {
    	imx_cli_print( "DCT internal destination: 0x%lx, size %lu \r\n",
			dct_header->boot_detail.load_details.destination.detail.internal_fixed.location,
			dct_header->boot_detail.load_details.destination.detail.internal_fixed.size );
    }
    else {
    	imx_cli_print( "Invalid boot destination.\r\n" );
    }
    imx_cli_print( "Load app function address 0x%lx \r\n", (unsigned int)dct_header->load_app_func );
    uint16_t i;
    imx_cli_print( "Saved App Locations:\r\n" );
    for ( i = 0; i < DCT_MAX_APP_COUNT; i++ ) {
    	if ( dct_header->apps_locations[ i ].id == EXTERNAL_FIXED_LOCATION ) {
			imx_cli_print( "(%u) 0x%lx, size: %lu \r\n", i,
				dct_header->apps_locations[ i ].detail.external_fixed.location,
				dct_header->apps_locations[ i ].detail.external_fixed.size );
    	}
    	else {
    		imx_cli_print( "(%u) Entry is not listed as external fixed.\r\n");
    	}
    }

    /* Here ptr_is_writable should be same as what we passed during wiced_dct_read_lock() */
    wiced_dct_read_unlock( dct_header, WICED_FALSE );

    return WICED_SUCCESS;
}

wiced_result_t print_security_dct( void )
{
    platform_dct_security_t* dct_security = NULL;

    if ( wiced_dct_read_lock( (void**) &dct_security, WICED_FALSE, DCT_SECURITY_SECTION, 0, sizeof( *dct_security ) ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    /* since we passed ptr_is_writable as WICED_FALSE, we are not allowed to write in to memory pointed by dct_security */

    imx_cli_print( "\r\n----------------------------------------------------------------\r\n\r\n");

    /* Security Section */
    imx_cli_print( "Security Section \r\n");
    imx_cli_print( "    Certificate : \r\n%s \r\n", dct_security->certificate );
    imx_cli_print( "    Private Key : \r\n%s \r\n", dct_security->private_key );

    /* Here ptr_is_writable should be same as what we passed during wiced_dct_read_lock() */
    wiced_dct_read_unlock( dct_security, WICED_FALSE );

    return WICED_SUCCESS;
}


wiced_result_t print_mfg_info_dct( void )
{
    platform_dct_mfg_info_t* dct_mfg_info = NULL;

    if ( wiced_dct_read_lock( (void**) &dct_mfg_info, WICED_FALSE, DCT_MFG_INFO_SECTION, 0, sizeof( *dct_mfg_info ) ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    /* since we passed ptr_is_writable as WICED_FALSE, we are not allowed to write in to memory pointed by dct_security */

    imx_cli_print( "\r\n----------------------------------------------------------------\r\n\r\n");

    /* Manufacturing Info Section */
    imx_cli_print( "Manufacturing Info Section \r\n");
    imx_cli_print( "     manufacturer          : %s \r\n", dct_mfg_info->manufacturer );
    imx_cli_print( "     product_name          : %s \r\n", dct_mfg_info->product_name );
    imx_cli_print( "     BOM_name              : %s \r\n", dct_mfg_info->BOM_name );
    imx_cli_print( "     BOM_rev               : %s \r\n", dct_mfg_info->BOM_rev );
    imx_cli_print( "     serial_number         : %s \r\n", dct_mfg_info->serial_number );
    imx_cli_print( "     manufacture_date_time : %s \r\n", dct_mfg_info->manufacture_date_time );
    imx_cli_print( "     manufacture_location  : %s \r\n", dct_mfg_info->manufacture_location );
    imx_cli_print( "     bootloader_version    : %s \r\n", dct_mfg_info->bootloader_version );

    /* Here ptr_is_writable should be same as what we passed during wiced_dct_read_lock() */
    wiced_dct_read_unlock( dct_mfg_info, WICED_FALSE );

    return WICED_SUCCESS;
}


wiced_result_t print_wifi_config_dct( void )
{
    platform_dct_wifi_config_t* dct_wifi_config = NULL;

    if ( wiced_dct_read_lock( (void**) &dct_wifi_config, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, 0, sizeof( *dct_wifi_config ) ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    /* since we passed ptr_is_writable as WICED_FALSE, we are not allowed to write in to memory pointed by dct_security */

    imx_cli_print( "\r\n----------------------------------------------------------------\r\n\r\n");

    /* Wi-Fi Config Section */
    imx_cli_print( "Wi-Fi Config Section \r\n");
    imx_cli_print( "    device_configured               : %d \r\n", dct_wifi_config->device_configured );
    imx_cli_print( "    stored_ap_list[0]  (SSID)       : %s \r\n", dct_wifi_config->stored_ap_list[0].details.SSID.value );
    imx_cli_print( "    stored_ap_list[0]  (Passphrase) : %s \r\n", dct_wifi_config->stored_ap_list[0].security_key );
    imx_cli_print( "    soft_ap_settings   (SSID)       : %s \r\n", dct_wifi_config->soft_ap_settings.SSID.value );
    imx_cli_print( "    soft_ap_settings   (Passphrase) : %s \r\n", dct_wifi_config->soft_ap_settings.security_key );
    imx_cli_print( "    config_ap_settings (SSID)       : %s \r\n", dct_wifi_config->config_ap_settings.SSID.value );
    imx_cli_print( "    config_ap_settings (Passphrase) : %s \r\n", dct_wifi_config->config_ap_settings.security_key );
    imx_cli_print( "    country_code                    : %c%c%d \r\n", ((dct_wifi_config->country_code) >>  0) & 0xff,
                                                                            ((dct_wifi_config->country_code) >>  8) & 0xff,
                                                                            ((dct_wifi_config->country_code) >> 16) & 0xff);
    imx_cli_print( "    DCT mac_address                 : ");
    print_my_mac_address( (wiced_mac_t*) &dct_wifi_config->mac_address );
    imx_cli_print("\r\n");

    /* Here ptr_is_writable should be same as what we passed during wiced_dct_read_lock() */
    wiced_dct_read_unlock( dct_wifi_config, WICED_FALSE );

    return WICED_SUCCESS;
}

/**
 * Print the App section of the DCT.
 *
 * written by Eric Thelin 2 November 2016
 */
wiced_result_t print_app_dct( void )
{
	// Get pointers to the App section's header and security key

    device_app_dct_t* apps_dct = wiced_dct_get_current_address( DCT_APP_SECTION );// These DCT pointers are only valid until a write is done to the DCT.
	IOT_Device_Config_t *config = wiced_dct_get_current_address( DCT_APP_SECTION ) + OFFSETOF( device_app_dct_t, config );

    imx_cli_print( "\r\n----------------------------------------------------------------\r\n\r\n");
    imx_cli_print( "Application DCT Starts @: 0x%08lx, Config: 0x%08lx\r\n", (uint32_t) apps_dct, (uint32_t) config );
    imx_cli_print( "Application Section DCT version: %u, reboots: %u OTA App version: %u\r\n", apps_dct->header.dct_version & 0xFF, config->reboots, apps_dct->header.ota_app_version );
    imx_cli_print( "System Image is %s and in the %s location.\r\n",
    		( apps_dct->header.changed_known_good.system_image == apps_dct->header.using_alternate.system_image ) ? "known good" : "transitional",
    		apps_dct->header.using_alternate.system_image ? "alternate" : "normal" );
    imx_cli_print( "OTA App Image is %s and in the %s location.\r\n",
    		( apps_dct->header.changed_known_good.ota_image == apps_dct->header.using_alternate.ota_image ) ? "known good" : "transitional",
    		apps_dct->header.using_alternate.ota_image ? "alternate" : "normal" );
    imx_cli_print( "PSOC Image is %s and in the %s location.\r\n",
    		( apps_dct->header.changed_known_good.psoc_image == apps_dct->header.using_alternate.psoc_image ) ? "known good" : "transitional",
    		apps_dct->header.using_alternate.psoc_image ? "alternate" : "normal" );

    return WICED_SUCCESS;
}

wiced_result_t print_network_config_dct( void )
{
    platform_dct_network_config_t* dct_network_config = NULL;

    if ( wiced_dct_read_lock( (void**) &dct_network_config, WICED_FALSE, DCT_NETWORK_CONFIG_SECTION, 0, sizeof( *dct_network_config ) ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    /* since we passed ptr_is_writable as WICED_FALSE, we are not allowed to write in to memory pointed by dct_security */

    imx_cli_print( "\r\n----------------------------------------------------------------\r\n\r\n");

    /* Network Config Section */
    imx_cli_print( "Network Configuration Section\r\n");
    imx_cli_print( "    wiced_interface_t       : %u \r\n", dct_network_config->interface  );
    imx_cli_print( "    hostname                : %s \r\n", dct_network_config->hostname.value );


    /* Here ptr_is_writable should be same as what we passed during wiced_dct_read_lock() */
    wiced_dct_read_unlock( (void*) dct_network_config, WICED_FALSE );

    return WICED_SUCCESS;
}

