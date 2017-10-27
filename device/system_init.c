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
/** @file init.c
 *
 *
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "config.h"
#include "hal_leds.h"
#include "icb_def.h"
#include "var_data.h"
#include "system_init.h"
#include "device.h"
#include "config.h"
#include "../cli/cli.h"
#include "../cli/interface.h"
#include "../cli/telnetd.h"
#include "../cs_ctrl/common_config.h"
#include "../location/location.h"
#include "../sflash/sflash.h"

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
 *               Function Declarations
 ******************************************************/
void mutex_init(void);
void create_msg_lists(void);
/******************************************************
 *                    Structures
 ******************************************************/
extern iMatrix_Control_Block_t icb;
extern IOT_Device_Config_t device_config;
/******************************************************
 *               Variable Definitions
 ******************************************************/
/*
 * For CoAP Networking
 */
uint16_t message_id;
uint32_t request_id;    // Used as our token
unsigned int random_seed; //This is the only definition, see coap.h for the extern declaration
wiced_semaphore_t wiced_semaphore;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
 * Check to make sure the current version of the DCT is being used. If not update it.
 * Somewhat large data blocks of stack space are used for the DCT write,
 * so this function should only be used in application_start() where the full stack is available.
 * Because this function ensures the DCT is valid, it should be called before any functions that access
 * the DCT like functions that connect to the WiFi.
 *
 * written by Eric Thelin 2 November 2016
 */
void verify_dct_and_update_if_needed(void)
{
	// This location becomes invalid after the first DCT write operation.
//    uint8_t *temp_version = (uint8_t*)wiced_dct_get_current_address( DCT_APP_SECTION ) + OFFSETOF( device_app_dct_t, dct_version );
    /*
     * Verify that this DCT is the correct format. If not update it and save
     */
// The following function upgraded to an out of date DCT structure.
//    update_dct_app_section( *temp_version );// This is a DCT write operation which is only executed if the version of the DCT is wrong.
}
/**
  * @brief	initialize the system
  * @param  None
  * @retval : True - Success / False - Failure
  */
bool system_init(void)
{
	wiced_result_t wiced_result;
	wiced_utc_time_t now;

    imx_printf( "Preparing WICED...");

    wiced_result = wiced_init();  //wiced_core_init();
	if ( wiced_result != WICED_SUCCESS ) {
		imx_printf( "wiced_core_init() failed with error code: %u.\n", wiced_result );
		return false;
	}

    imx_printf( "Commencing Setup procedure: " );


#ifdef USE_CCMRAM
    imx_printf( " *** Using CCMRAM ***\r\n" );
#else
    imx_printf( "\r\n" );
#endif
    // Ensure the DCT is valid before anything needs to access the DCT like functions that connect to the Wi-Fi.

    verify_dct_and_update_if_needed();
	device_init();
    /*
     * Set up a semaphore to use for WICED accesses
     */
    wiced_result = wiced_rtos_init_semaphore( &wiced_semaphore );
    if( wiced_result != WICED_SUCCESS )
        imx_printf( "Failed to initialize WICED Semaphore\r\n" );
    wiced_result = wiced_rtos_set_semaphore( &wiced_semaphore );    // Get it ready to use
    if( wiced_result != WICED_SUCCESS )
        imx_printf( "Failed to set WICED Semaphore\r\n" );

    mutex_init();

    create_msg_lists(); // Initialize CoAP message memory management lists.

    /*
     * Always print status messages? - dcb is initialized to all 0s
     */
#ifdef imx_printf_MSGS
    icb.print_debugs = true;
#endif
    /*
     * Set up a random starting message ID
     */
    random_seed = device_config.sn.serial1;
    message_id = rand_r( &random_seed );
    request_id = rand_r( &random_seed );

    icb.fake_utc_boot_time = (wiced_utc_time_t) ( device_config.last_ntp_updated_time / 1000 );// Convert ms to seconds
    wiced_time_set_utc_time_ms( &( device_config.last_ntp_updated_time ) );
    icb.boot_time = icb.fake_utc_boot_time;

	if( init_serial_flash() == false )
	    imx_printf( "ERROR: Serial Flash size does not match product definition\r\n" );
    device_config.boot_count += 1;
    imatrix_save_config();

    imx_printf( "Core System Initialized\r\n" );


    telnetd_init();
	cli_init();
    // Ensure that it will be night time till Networking is successful.

    wiced_time_get_utc_time( &now );
    icb.sunrise = icb.sunset = now + 3600;// Make sunrise an hour after the current time.
    /*
     * Start with last known location
     */
    icb.latitude = device_config.latitude;
    icb.longitude = device_config.longitude;
    icb.elevation = device_config.elevation;
    /*
     * Initialize sensors and controls - Check if none are defined, then we are probably starting with a blank config - Initialize the system
     */
    imx_printf( "Setting up %s, Product ID: 0x%08lx, Serial Number: %s\r\n", device_config.product_name, device_config.product_id, device_config.device_serial_number );
    /*
     * Set up variable length payload pools
     */
    init_var_pool();

    imx_printf( "System has %u Controls and %u Sensors\r\n", device_config.no_controls, device_config.no_sensors );

    imx_printf( "Initializing Controls & Sensors\r\n" );
    cs_init();

    // imx_printf( "Configuring Smart Arduino\r\n" );
    // configure_arduino();
    imx_printf( "Initializing Locaton System\r\n" );

    init_location_system();

    icb.wifi_state = MAIN_WIFI_SETUP;

    imx_printf( "Initialization Complete, Thing will run in %s mode\r\n", device_config.AP_setup_mode ? "Provisioning" : "Operational" );

	return true;
}
