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

/** @file imatrix.c
 *
 *  Created on: October 8, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "storage.h"
#include "imatrix_interface.h"
#include "cli/cli.h"
#include "cli/cli_status.h"
#include "cli/telnetd.h"
#include "cs_ctrl/hal_sample.h"
#include "coap/coap_receive.h"
#include "coap/coap_transmit.h"
#include "device/config.h"
#include "device/icb_def.h"
#include "device/imx_LEDS.h"
#include "device/system_init.c"
#include "imatrix_upload/imatrix_upload.h"
#include "ota_loader/ota_loader.h"
#include "ota_loader/ota_structure.h"
#include "time/watchdog.h"
#include "wifi/process_wifi.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
//#define TEST_STANDALONE
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
extern iMatrix_Control_Block_t icb;
imx_imatrix_init_config_t imx_imatrix_init_config;
#ifdef TEST_STANDALONE
#include "test_imatrix.h"
#endif
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Initialize the iMatrix System
  * @param  flag to set up to run in background
  * @retval : status of initialization
  */
imx_status_t imx_init( imx_imatrix_init_config_t *init_config, bool override_config, bool run_in_background )
{
    imx_status_t result;

    /*
     * Set up watchdog
     */
    imx_init_watchdog();

    memset( &icb, 0x00, sizeof( iMatrix_Control_Block_t ) );
    /*
     * Always print status messages? - dcb is initialized to all 0s - this is for diagnostic mode - comment out when not used
     */
    device_config.print_debugs = true;

    printf( "Commencing iMatrix Initialization Sequence\r\n" );
    /*
     * Save user defined product information to local storage
     */
    memcpy( &imx_imatrix_init_config, init_config, sizeof( imx_imatrix_init_config_t ) );

    /*
     * Set up LED functions as we use these to tell what is going on
     */
    imx_init_led_functions( init_config->led_functions );

    result = system_init( override_config );
    if( result != IMX_SUCCESS )
        return result;

    if( run_in_background ) {
        /*
         * Spawn the imx_process as a background process
         */
        icb.running_in_background = true;
    }
    /*
     * Done Setup Turn LEDS OFF
     */
    imx_set_led( 0, IMX_LED_ALL_OFF, 0 );

    return IMX_SUCCESS;
}
#ifdef TEST_STANDALONE
imx_imatrix_init_config_t test_imatrix_config = {
    .product_name = IMX_PRODUCT_NAME,
    .device_name = IMX_PRODUCT_NAME,
    .imatrix_public_url = IMX_IMATRIX_SITE,
    .ota_public_url = IMX_OTA_SITE,
    .manufacturing_url = IMX_MANUFACTURING_SITE,
    .no_sensors = IMX_NO_SENSORS,
    .no_controls = IMX_NO_CONTROLS,
    .no_arduino_sensors = IMX_NO_ARDUINO_SENSORS,
    .no_arduino_controls = IMX_NO_ARDUINO_CONTROLS,
    .no_at_controls = IMX_NO_AT_CONTROLS,
    .at_control_start = IMX_AT_CONTROL_START,
    .no_at_sensors = IMX_NO_AT_SENSORS,
    .at_sensor_start = IMX_AT_SENSOR_START,
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
    .latitude = IMX_LATITUDE_DEFAULT,
    .elevation = IMX_ELEVATION_DEFAULT
};
/**
  * @brief  run the iMatrix System
  * @param  None
  * @retval : status of initialization
  */

void application_start(void)
{
    imx_init( &test_imatrix_config, true, false );
    while( true )
        imx_process();
}
#endif
/**
  * @brief  Process the iMatrix System
  * @param  None
  * @retval : status of system
  */
imx_status_t imx_process(void)
{
    wiced_time_t current_time;

    wiced_time_get_time( &current_time );

    /*
     * Keep the watchdog happy
     */
    imx_kick_watchdog();

    if ( ota_is_active() ) {
        /*
         * Handle OTA functions ONLY
         */
        ota_loader();
        return IMX_SUCCESS;
    }

    if( ota_get_latest_is_active() ) {
        /*
         * Handle Get Latest
         */
        get_latest_version();
    }
    /*
     * Process CLI & Telnet Clients
     */
    cli_process();
    telnetd();
    /*
     * Process controls Controls are set by direct action from logic or from CoAP POST
     */
    hal_sample( IMX_CONTROLS, current_time );
    /*
     * Process Sensors
     */
    hal_sample( IMX_SENSORS, current_time );
    /*
     * Process Location system
     */
    process_location( current_time );
    /*
     * Process iMatrix Uploads
     */
    imatrix_upload( current_time );

    process_wifi( current_time );

    /*
     * Process any items on the CoAP queues
     */
    coap_recv( true );
    coap_transmit( true );

    return IMX_SUCCESS;
}
imx_status_t imx_deinit(void)
{
    if( icb.running_in_background == true ) {
        /*
         * Shut down background process
         */
        ;
    }
    return IMX_SUCCESS;
}

void imx_boot_factory_reset(void)
{
    /*
     * This should boot to factory reset image
     */
    imx_printf( "System Failed to initialize - restoring to know good software image\r\n" );
    imatrix_destroy_config();
    /*
     * For now boot to image 6
     */
    reboot_to_image( APP1 );
    /*
     * If we return from this then let the watchdog reboot this unit after we clear config data so system will do a full re intialization
     */
    imx_printf( "Factory Reset Image not valid - Attempting a fresh start with cleared configuration with currently installed Application\r\n" );
    while( 1 )
        ;
}
