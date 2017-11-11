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
/** @file cli_status.c
 *
 *
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdbool.h>
#include <ctype.h>
#include "wiced.h"

#include "../storage.h"
#include "interface.h"
#include "./ble/ble_manager.h"
#include "../device/config.h"
#include "../device/hal_wifi.h"
#include "../device/hal_leds.h"
#include "../device/icb_def.h"
#include "../device/imx_LEDS.h"
#include "../imatrix/imatrix.h"
#include "../wifi/wifi.h"
#include "cli_status.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define FEET_IN_1METER	    3.28084
#define VAR_PRINT_LENGTH    16
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
extern control_sensor_data_t sd[ MAX_NO_SENSORS ];
extern control_sensor_data_t cd[ MAX_NO_CONTROLS ];
/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief Print the status of the device
  * @param  None
  * @retval : None
  */
void cli_status( uint16_t arg )
{
	UNUSED_PARAMETER(arg);

	uint16_t i;
	uint32_t channel;
	int rssi, noise;
	wiced_time_t current_time;
	wiced_iso8601_time_t iso8601_time;
	wiced_utc_time_t utc_time;

    cli_print( "Product Name: %s, Device Name: %s - ", device_config.product_name, device_config.device_name  );
    cli_print( "Serial Number: %08lX%08lX%08lX - iMatrix assigned: %s\r\n", device_config.sn.serial1, device_config.sn.serial2, device_config.sn.serial3, device_config.device_serial_number );
    cli_print( "Last NTP Updated time: %lu, Reboot Counter: %lu, Valid Config: 0x%08x\r\n", (uint32_t) device_config.last_ntp_updated_time, device_config.reboots, device_config.valid_config );
	cli_print( "Device location: Longitude: %f, Latitude: %f, Elevation: %fm (%6.2fft.)\r\n", icb.longitude, icb.latitude, icb.elevation, ( icb.elevation * FEET_IN_1METER )  );
	wiced_time_get_utc_time( &utc_time );
	wiced_time_get_iso8601_time( &iso8601_time );
	cli_print( "System UTC time is: %lu -> %.26s Current Status:\r\n", utc_time, (char*)&iso8601_time );

	cli_print( "Device is: " );
	if( icb.wifi_up == true ) {
		cli_print( "Online, " );
        cli_print( "IP: %u.%u.%u.%u, ",
                (unsigned int)((GET_IPV4_ADDRESS( icb.my_ip ) >> 24) & 0xFF),
                (unsigned int)((GET_IPV4_ADDRESS( icb.my_ip ) >> 16) & 0xFF),
                (unsigned int)((GET_IPV4_ADDRESS( icb.my_ip ) >>  8) & 0xFF),
                (unsigned int)((GET_IPV4_ADDRESS( icb.my_ip ) >>  0) & 0xFF ) );
		if( device_config.AP_setup_mode == true ) {
			cli_print( "Access Point / Setup mode, SSID: %s, WPA2PSK: %s, ", device_config.ap_ssid, device_config.ap_wpa );
			/*
			 * Add Code to list clients
			 */
			cli_print( "Client list not yet implemented" );
		} else {
			rssi = hal_get_wifi_rssi();
			noise = hal_get_wifi_noise();
			channel = hal_get_wifi_channel();
    		cli_print( "Secured with: " );
		    switch( device_config.st_security_mode ) {
		    	case IMX_SECURITY_8021X_EAP_TLS :
		    		cli_print( "802.1X EAP TLS, " );
		    		break;
		    	case IMX_SECURITY_8021X_PEAP :
		    		cli_print( "802.1X PEAP TLS, " );
		    		break;
		    	case IMX_SECURITY_WEP :
		    		cli_print( "WEP, " );
		    		break;
		    	case IMX_SECURITY_WPA2 :
		    	default :
		    		cli_print( "WPA2PSK, " );
		    		break;
		    }
			cli_print( "Connected to SSID: %s, Channel: %lu, RSSI: %lddB, Noise: %lddB, S/N: %ddB", device_config.st_ssid, channel, rssi, noise, rssi - noise);
		}
	} else
		cli_print( "Offline" );
	cli_print( "\r\n" );
	for( i = 0; i < NO_PROTOCOL_STATS; i++ ) {
	    if( i == UDP_STATS )
	        cli_print( "    UDP Statistics:" );
	    else if( i == TCP_STATS )
            cli_print( "    TCP Statistics:" );
	    cli_print( " Packets - Received: %lu, Multicast: %lu, Unicast Received: %lu, Errors: %lu",
	            icb.ip_stats[ i ].packets_received, icb.ip_stats[ i ].packets_multicast_received, icb.ip_stats[ i ].packets_unitcast_received, icb.ip_stats[ i ].packets_received_errors );
	    cli_print( " Creation Failure: %lu, Fail to Send: %lu, Packets Sent: %lu, Last Receive Error: %lu\r\n",
	            icb.ip_stats[ i ].packet_creation_failure, icb.ip_stats[ i ].fail_to_send_packet, icb.ip_stats[ i ].packets_sent, icb.ip_stats[ i ].rec_error );
	}

    cli_print( "AT Commands processed: %lu, Errors: %lu, Verbose mode: ", icb.AT_commands_processed, icb.AT_command_errors );
    switch( device_config.AT_verbose ) {
    case IMX_AT_VERBOSE_NONE :
        cli_print( "None" );
        break;
    case IMX_AT_VERBOSE_STANDARD :
        cli_print( "AT Commands & CLI Responses ONLY" );
        break;
    case IMX_AT_VERBOSE_STANDARD_STATUS :
        cli_print( "AT Responses & Status Messages" );
        break;
    }

    cli_print( "\r\n" );
	wiced_time_get_time( &current_time );
	/*
	 * Display status of controls
	 */
	peripheral_type_t type;
	control_sensor_data_t *csd;
	imx_control_sensor_block_t *cs_block;
	uint16_t no_items;

	for( type = 0; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
	    if( type == IMX_CONTROLS ) {
	        cs_block = &device_config.ccb[ 0 ];
	        csd = &cd[ 0 ];
	    } else {
	        cs_block = &device_config.scb[ 0 ];
            csd = &sd[ 0 ];
	    }
        cli_print( "%u %s: Current Status @: %lu mSec\r\n", ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors, ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", current_time );
        no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

        for( i = 0; i < no_items; i++ ) {
            if( cs_block[ i ].enabled == true ) {
                cli_print( "  No: %2u, %32s, ID: 0x%08lx, ", i, cs_block[ i ].name, cs_block[ i ].id );
                if( cs_block[ i ].valid == true ) {
                    cli_print( "Current setting: " );
                    switch( cs_block[ i ].data_type ) {
                        case IMX_UINT32 :
                            cli_print( "0x%08lx - %lu", csd[ i ].last_value.uint_32bit, csd[ i ].last_value.uint_32bit );
                            break;
                        case IMX_INT32 :
                            cli_print( "%ld", csd[ i ].last_value.int_32bit );
                            break;
                        case IMX_FLOAT :
                            cli_print( "%0.6f", csd[ i ].last_value.float_32bit );
                            break;
                        case IMX_VARIABLE_LENGTH :
                            print_var_data( VR_DATA_MAC_ADDRESS, csd[ i ].last_value.var_data );
                            break;
                    }
                    cli_print( ", " );
                    switch( cs_block[ i ].data_type ) {
                        case IMX_UINT32 :
                            cli_print( "32 Bit Unsigned" );
                            break;
                        case IMX_INT32 :
                            cli_print( "32 Bit signed" );
                            break;
                        case IMX_FLOAT :
                            cli_print( "32 Bit Float" );
                            break;
                    }
                    cli_print( ", Errors: %lu, ", cd[ i ].errors );
                    if( cs_block[ i ].sample_rate == 0 )
                        cli_print( "Event Driven" );
                    else {
                        if( cs_block[ i ].sample_rate >= 1000 )
                            cli_print( "Sample Every: %4.1f Sec", ( (float) cs_block[ i ].sample_rate ) / 1000.0 );
                        else
                            cli_print( "Sample Every: %5u mSec", cs_block[ i ].sample_rate );
                    }
                } else
                    cli_print( "No Data Recorded");
                cli_print( "\r\n" );
            }
        }
	}

	print_led_status();
	/*
	 * BLE Scan Status
	 */
//	print_ble_scan_results( 1 );

}

void print_var_data( var_data_types_t data_type, var_data_entry_t *var_data )
{
    uint16_t i;
    wiced_mac_t *bssid;

    if( var_data == NULL ) {
        cli_print( "None" );
        return;
    }

    switch( data_type ) {
        case VR_DATA_STRING :
            cli_print( " String: %s", (char *) var_data->data );
            break;
        case VR_DATA_MAC_ADDRESS :
            bssid = ( wiced_mac_t *) var_data->data;
            cli_print( " BSSID: %02x:%02x:%02x:%02x:%02x:%02x", bssid->octet[ 0 ], bssid->octet[ 1 ], bssid->octet[ 2 ], bssid->octet[ 3 ],
                            bssid->octet[ 4 ], bssid->octet[ 5 ] );
            break;
        default :
            /*
             * Just print up to the first 16 Characters as Hex and Char
             */
            for( i = 0; ( ( i < VAR_PRINT_LENGTH ) && ( i < var_data->header.length ) ); i++ )
                cli_print( " 0x%02X", var_data->data[ i ] );
            cli_print( "  " );
            for( i = 0; ( ( i < VAR_PRINT_LENGTH ) && ( i < var_data->header.length ) ); i++ )
                cli_print( " %c", ( isprint( (int) var_data->data[ i ] ) == true ) ? (char) var_data->data[ i ] : '*' );
            break;
    }

}
