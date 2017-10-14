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
 *
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "wiced.h"

#include "../defines.h"
#include "../system.h"
#include "../hal.h"
#include "../cli/interface.h"
#include "../device/dcb_def.h"
#include "../device/config.h"
#include "../networking/keep_alive.h"
#include "../time/ck_time.h"
#include "wifi.h"
#include "process_wifi.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define KEEP_ALIVE_PERIOD_MSEC                  (70000) // Every 70 seconds with between 70 and 140 seconds before the first keep alive
#define KEEP_ALIVE_ALREADY_STARTED              (0xFFFFFFFF) // The back off is only used before starting Keep Alive. Once started use this value for backoff.

#define MAXIMUM_TIME_TILL_WATCHDOG_BITES_PER_THREAD ( 60000*MILLISECONDS )
#define WIFI_CHECK_INTERVAL	( 60 * 1000 )	// 60 Seconds check

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
extern dcb_t dcb;
/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
uint16_t wifi_check_count = 0, wifi_was_connected = false;
uint32_t random_seed_from_mac = 0, keep_alive_backoff = 0;
wiced_time_t last_wifi_check = 0, wifi_up_time = 0, start_time_synch = 0;
wiced_utc_time_t utc_time = 0, sec_since_boot = 0;

extern dcb_t dcb;
extern unsigned int random_seed;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	process the Wi Fi and timing issues
  * @param  None
  * @retval : None
  */
void process_wifi(wiced_time_t current_time )
{
	wiced_mac_t  mac;

	wifi_was_connected = false;

	switch( dcb.wifi_state ) {
		case MAIN_WIFI_SETUP :
		default :
// - Add support for watchdog later					wiced_update_system_monitor( &watchdog, MAXIMUM_TIME_TILL_WATCHDOG_BITES_PER_THREAD );
		    print_status( "Trying to Init Wi Fi\r\n" );
			if ( wifi_init() == true ) {
				wifi_was_connected = true;
				/*
				 * Set up the RTC
				 */
				if( device_config.AP_setup_mode == false ) {	// We should be connected to the Internet
					wiced_time_get_time( &start_time_synch );
					wiced_time_get_utc_time( &utc_time );
					sec_since_boot = utc_time - dcb.fake_utc_boot_time;

					    print_status( "\r\nRTC Start.....\r\n" );
				    start_random_delay_timer_for_sntp();
				}
			    wwd_wifi_get_mac_address( &mac, WWD_STA_INTERFACE );
				random_seed_from_mac = ( mac.octet[0] << 20 ) | (mac.octet[1] << 16 ) | ( mac.octet[2] << 12 ) | ( mac.octet[3] << 8 ) | ( mac.octet[4] << 4 ) | mac.octet[5];
              	random_seed = device_config.sn.serial1;
                keep_alive_backoff = floor( ( (double)KEEP_ALIVE_PERIOD_MSEC + 1 ) * ( (double)rand_r( &random_seed )/( (double)RAND_MAX + 1 ) ) );
				wiced_time_get_time( &wifi_up_time );
				wifi_check_count = 0;
        		wiced_time_get_time( &current_time );
				dcb.wifi_state = MAIN_WIFI;
			} else {
				last_wifi_check = current_time;
				wifi_check_count += 1;
				cli_print( "Retry Count: %u\r\n", wifi_check_count );
				if( wifi_check_count >= 120 && wifi_was_connected == true ) {	// 2hrs minutes - reboot - but only if we had Wi Fi was here
					cli_print( "Rebooting due to Wi Fi Failure\r\n" );
					while( 1 )
						;						// Watchdog will bite
				}
				dcb.wifi_state = MAIN_NO_WIFI;
			}
			break;
		case MAIN_NO_WIFI :
//					kick_watchdog_and_do_everything( current_time, &watchdog, MAXIMUM_TIME_TILL_WATCHDOG_BITES_PER_THREAD );
			if( timer_timeout( current_time, last_wifi_check, WIFI_CHECK_INTERVAL ) ) {
				print_status( "Checking for Wi Fi ...\r\n" );
				dcb.wifi_state = MAIN_WIFI_SETUP;
			}
			break;
		case MAIN_WIFI :
			if ( device_config.AP_setup_mode == false ) {

				start_stopped_auto_time_sync_if_random_delay_is_finished();

				// Calculate the correct boot time the first time NTP succeeds.

			    if ( ( dcb.boot_time == dcb.fake_utc_boot_time ) && ntp_succeeded_at_least_once() ) {
					wiced_time_t time;
					wiced_time_get_time( &time );
					wiced_time_get_utc_time( &utc_time );

					dcb.boot_time = utc_time - sec_since_boot - ( time_difference( time, start_time_synch )/1000 );
			    }

				// Start keep alive when backoff expires.

				if ( ( keep_alive_backoff != KEEP_ALIVE_ALREADY_STARTED ) && ( is_later( current_time, wifi_up_time + keep_alive_backoff ) ) ) {
				    start_keep_alive( KEEP_ALIVE_PERIOD_MSEC );
					keep_alive_backoff = KEEP_ALIVE_ALREADY_STARTED;
				}

			}

//					kick_watchdog_and_do_everything( current_time, &watchdog, MAXIMUM_TIME_TILL_WATCHDOG_BITES_PER_THREAD );
			if( dcb.wifi_up == false ) {
				print_status( "Wi Fi has dropped\r\n" );
				/*
				 * Wi Fi Gone away
				 *
				 */
				stop_network();
				keep_alive_backoff = 0;
				dcb.wifi_state = MAIN_WIFI_SETUP;
				print_status( "Trying to restart Network\r\n" );
			}
			break;
	}
}
