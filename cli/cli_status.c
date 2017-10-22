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

#include "wiced.h"

#include "../storage.h"
#include "interface.h"
#include "./ble/ble_manager.h"
#include "../device/config.h"
#include "../device/hal_wifi.h"
#include "../device/hal_leds.h"
#include "../device/icb_def.h"
#include "../platform_functions/ISMART_LEDS.h"
#include "../imatrix/imatrix.h"
#include "../wifi/wifi.h"
#include "cli_status.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define FEET_IN_1METER	3.28084
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

	cli_print( "Running WICED: %s, Name: %s, ", WICED_VERSION, device_config.product_name );
	cli_print( "Serial Number: %s (%08lX%08lX%08lX) - %s\r\n", device_config.device_serial_number,
			device_config.sn.serial1, device_config.sn.serial2, device_config.sn.serial3, device_config.provisioned == true ? "Provisioned" : "Not provisioned" );
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

    cli_print( "AT Commands processed: %lu, Errors: %lu, ", icb.AT_commands_processed, icb.AT_command_errors );
    //print_led_status();
	// print_controls();
	// print_sensors();

	wiced_time_get_time( &current_time );
	/*
	 * Display status of controls
	 */
    cli_print( "Current Controls Status @ %lu:\r\n", current_time );
	for( i = 0; i < device_config.no_controls; i++ ) {
	    if( device_config.ccb[ i ].enabled == true ) {
	        cli_print( "Control no: %u, %s, ID: 0x%08lx, ", i, device_config.ccb[ i ].name, device_config.ccb[ i ].id );
            cli_print( "Current setting: " );
            switch( device_config.ccb[ i ].data_type ) {
                case IMX_DO_UINT32 :
                    cli_print( "0x%08lx - %lu", cd[ i ].last_value.uint_32bit, cd[ i ].last_value.uint_32bit );
                    break;
                case IMX_DO_INT32 :
                    cli_print( "%ld", cd[ i ].last_value.int_32bit );
                    break;
                case IMX_AO_FLOAT :
                    cli_print( "%0.6f", cd[ i ].last_value.float_32bit );
                    break;
            }
            cli_print( ", " );
	        switch( device_config.ccb[ i ].data_type ) {
	            case IMX_DO_INT32 :
	                cli_print( "32 Bit signed" );
	                break;
	            case IMX_DO_UINT32 :
	                cli_print( "32 Bit Unsigned" );
	                break;
	            case IMX_AO_FLOAT :
	                cli_print( "32 Bit Float" );
	                break;
	        }
	        cli_print( ", Errors: %lu, ", cd[ i ].errors );
	        if( device_config.ccb[ i ].sample_rate == 0 )
	            cli_print( ", Event Driven" );
	        else
	            cli_print( ", Refresh rate: %u mS", device_config.ccb[ i ].sample_rate );
	        cli_print( "\r\n" );
	    }
	}
	/*
	 * Display Sensor data - including settings for alarms
	 */
	cli_print( "Current Sensor Status @ %lu:\r\n", current_time );
	for( i = 0; i < device_config.no_sensors; i++ ) {
        if( device_config.scb[ i ].enabled == true ) {
            cli_print( "Sensor no: %u, %s ID: 0x%08lx, - Last Value: ", i, device_config.scb[ i].name, device_config.scb[ i].id );
            switch( device_config.scb[ i ].data_type ) {
                case IMX_DI_UINT32 :
                    cli_print( "%lu - 32 Bit Unsigned", sd[ i ].last_value.uint_32bit );
                    break;
                case IMX_DI_INT32 :
                    cli_print( "%ld - 32 Bit Signed", sd[ i ].last_value.int_32bit );
                    break;
                case IMX_AI_FLOAT :
                    cli_print( "%f - 32 Bit Float", sd[ i ].last_value.float_32bit );
                    break;
            }
            cli_print( ", Errors: %lu,", sd[ i ].errors );
            cli_print( " Sample Batch: %u Samples, ", device_config.scb[ i ].sample_batch_size );
            if( device_config.scb[ i ].sample_rate == 0 )
                cli_print( "Event Driven" );
            else {
                cli_print( "Save Sample Rate Every: %u mS", device_config.scb[ i ].sample_rate );
                cli_print( "saving next sample in %lu mS",
                    (uint32_t) ( (uint32_t) device_config.scb[ i ].sample_rate - ( ( current_time - sd[ i ].last_sample_time ) / 1000 ) ) );
            }
            cli_print( ", %u Samples saved, last sample @: %lu\r\n", sd[ i ].no_samples, sd[ i ].last_sample_time  );

            /*
            cli_print( "Monitoring Levels low enabled:" );
            if( device_config.scb[ i ].use_warning_level_low == 0 )
                cli_print( " None" );
            else {
                cli_print( "%s%s%s",
                        ( ( device_config.scb[ i ].use_warning_level_high & USE_WARNING_LEVEL_1 ) != 0 ) ? " Watch" : " ",
                        ( ( device_config.scb[ i ].use_warning_level_high & USE_WARNING_LEVEL_2 ) != 0 ) ? " Advisory" : " ",
                        ( ( device_config.scb[ i ].use_warning_level_high & USE_WARNING_LEVEL_3 ) != 0 ) ? " Warning" : " " );
                cli_print( ", High Level Settings: Watch_level: ");
                switch( device_config.scb[ i ].data_type ) {
                    case DI_UINT32 :
                        cli_print( "%lu", device_config.scb[ i ].warning_level_high[ 0 ].uint_32bit );
                        break;
                    case DI_INT32 :
                        cli_print( "%ld", device_config.scb[ i ].warning_level_high[ 0 ].int_32bit );
                        break;
                    case AI_FLOAT :
                        cli_print( "%f", device_config.scb[ i ].warning_level_high[ 0 ].float_32bit );
                        break;
                }
            }
            cli_print( " Monitoring Levels high enabled:" );
            if( device_config.scb[ i ].use_warning_level_high == 0 )
                cli_print( " None" );
            else {
                cli_print( "%s%s%s",
                        ( ( device_config.scb[ i ].use_warning_level_high & USE_WARNING_LEVEL_1 ) != 0 ) ? " Watch" : " ",
                        ( ( device_config.scb[ i ].use_warning_level_high & USE_WARNING_LEVEL_2 ) != 0 ) ? " Advisory" : " ",
                        ( ( device_config.scb[ i ].use_warning_level_high & USE_WARNING_LEVEL_3 ) != 0 ) ? " Warning" : " " );
                cli_print( " Low Level Settings: Watch: ");
                switch( device_config.scb[ i ].data_type ) {
                    case DI_UINT32 :
                        cli_print( "%lu", device_config.scb[ i ].warning_level_low[ 0 ].uint_32bit );
                        break;
                    case DI_INT32 :
                        cli_print( "%ld", device_config.scb[ i ].warning_level_low[ 0 ].int_32bit );
                        break;
                    case AI_FLOAT :
                        cli_print( "%f", device_config.scb[ i ].warning_level_low[ 0 ].float_32bit );
                        break;
                }
                cli_print( ", Advisory: ");
                switch( device_config.scb[ i ].data_type ) {
                    case DI_UINT32 :
                        cli_print( "%lu", device_config.scb[ i ].warning_level_low[ 1 ].uint_32bit );
                        break;
                    case DI_INT32 :
                        cli_print( "%ld", device_config.scb[ i ].warning_level_low[ 1 ].int_32bit );
                        break;
                    case AI_FLOAT :
                        cli_print( "%f", device_config.scb[ i ].warning_level_low[ 1 ].float_32bit );
                        break;
                }
                cli_print( ", Warning: ");
                switch( device_config.scb[ i ].data_type ) {
                    case DI_UINT32 :
                        cli_print( "%lu", device_config.scb[ i ].warning_level_low[ 2 ].uint_32bit );
                        break;
                    case DI_INT32 :
                        cli_print( "%ld", device_config.scb[ i ].warning_level_low[ 2 ].int_32bit );
                        break;
                    case AI_FLOAT :
                        cli_print( "%f", device_config.scb[ i ].warning_level_low[ 2 ].float_32bit );
                        break;
                }
            }
            cli_print( "\r\n" );
            */
        }
	}
	print_led_status();
	/*
	 * BLE Scan Status
	 */
//	print_ble_scan_results( 1 );

}
