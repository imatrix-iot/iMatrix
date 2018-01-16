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

/** @file wifi_loggin.c
 *
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../host_support.h"
#include "../cli/interface.h"
#include "../cs_ctrl/imx_cs_interface.h"
#include "../device/icb_def.h"
#include "../device/hal_wifi.h"
#include "../device/var_data.h"
#include "../imatrix_upload/logging.h"

#include "wifi_logging.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define BSSID_STRING_LENGTH     20
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
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Log wth Wi Fi connection details
  *
  * @param  None
  * @retval : completion
  */

void log_wifi_connection(void)
{
    int32_t channel, noise, rssi;
    wiced_mac_t ap_bssid;
    var_data_entry_t *var_data_ptr;

    if( ( icb.wifi_up == false ) || ( device_config.AP_setup_mode == true ) )
        return;     // Nothing to log

    if( device_config.log_wifi_AP == true ) {
//        imx_printf( "Logging Wi Fi Connection:" );
        /*
         * Add Channel
         */
        channel = hal_get_wifi_channel();
//        imx_printf( " Channel: %ld", channel );
        imx_set_control_sensor( IMX_SENSORS, imx_get_wifi_channel_scb(), &channel );
        /*
         * Add BSSID Address
         */
        hal_get_wifi_bssid( &ap_bssid );
        var_data_ptr = imx_get_var_data( BSSID_STRING_LENGTH );
        if( var_data_ptr != NULL ) {
            sprintf( (char *) var_data_ptr->data, "%02x:%02x:%02x:%02x:%02x:%02x", ap_bssid.octet[ 0 ], ap_bssid.octet[ 1 ], ap_bssid.octet[ 2 ], ap_bssid.octet[ 3 ],
                    ap_bssid.octet[ 4 ], ap_bssid.octet[ 5 ] );
            var_data_ptr->length = strlen( (char *) var_data_ptr->data );    // Include NULL in the length of this data
//            imx_printf( " BSSID: %s", var_data_ptr->data );
            imx_set_control_sensor( IMX_SENSORS, imx_get_wifi_bssid_scb(), &var_data_ptr );
            /*
             * Free up buffer now it has been recorded
             */
            imx_add_var_free_pool( var_data_ptr );
        } else
            imx_printf( "Unable to get variable length data buffer\r\n" );
    }

    if( device_config.log_wifi_rssi == true ) {
        rssi = hal_get_wifi_rssi();
//        imx_printf( " RSSI: %ld", rssi );
        imx_set_control_sensor( IMX_SENSORS, imx_get_wifi_rssi_scb(), (void *) &rssi );
    }

    if( device_config.log_wifi_rfnoise == true ) {
        noise = hal_get_wifi_noise();
//        imx_printf( " RF Noise: %ld", noise );
        imx_set_control_sensor( IMX_SENSORS, imx_get_wifi_rf_noise_scb(), (void *) &noise );
    }

//    imx_printf( "\r\n" );

}

/**
 * After a call to network_up(), wiced_network_resume_after_deep_sleep() or join_ent(),
 * use this function to send iMatrix the total number of failed WiFi join attempts and successful ones
 * since the last time this function was called.
 * Additionally, the icb.wifi_(failed||success)_connect_count counters are incremented appropriately.
 *
 * written by Eric Thelin 30 November 2017
 */

void log_wifi_join_event_results( void )
{
#define WIFI_MSG_FORMAT_STRING "Wi Fi Failed to Connect %lu Times and Connected %lu Times"

    char msg[ strlen( WIFI_MSG_FORMAT_STRING ) + 7 * 2 + 1 ]; // Add 7 digits/uint32 and 1 NULL terminator
    uint32_t joins = wiced_successful_wifi_joins();
    uint32_t failed_joins = wiced_failed_wifi_joins();

    wiced_reset_wifi_event_counters( WICED_SUCCESSFUL_WIFI_JOIN_EVENT_COUNTER | WICED_FAILED_WIFI_JOIN_EVENT_COUNTER ); // Reset all counters to 0.

    sprintf( msg, WIFI_MSG_FORMAT_STRING, failed_joins, joins );
    log_iMatrix( msg );
    printf( "WIFI LOG: %s\r\n", msg );

    icb.wifi_failed_connect_count += failed_joins;
    icb.wifi_success_connect_count += joins;
}
