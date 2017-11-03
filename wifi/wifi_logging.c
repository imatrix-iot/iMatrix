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

#include "wifi_logging.h"

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
    var_data_entry_t *var_data_ptr;

    if( ( icb.wifi_up == false ) || ( device_config.AP_setup_mode == true ) )
        return;     // Nothing to log

    if( device_config.log_wifi_AP == true ) {
        imx_printf( "Logging Wi Fi Connection:" );
        /*
         * Add Channel
         */
        channel = hal_get_wifi_channel();
        imx_printf( " Channel: %ld", channel );
        imx_set_sensor( imx_get_wifi_channel_scb(), &channel );
        /*
         * This is variable length data, get a buffer to put the data in
         */
        var_data_ptr = get_var_data( sizeof( wiced_mac_t ) );
        if( var_data_ptr != NULL ) {
            /*
             * Add BSSID Address
             */
            hal_get_wifi_bssid( ( wiced_mac_t *) var_data_ptr->data );
            var_data_ptr->header.length = sizeof( wiced_mac_t );
            imx_printf( " BSSID: %02x:%02x:%02x:%02x:%02x:%02x", var_data_ptr->data[ 0 ], var_data_ptr->data[ 1 ], var_data_ptr->data[ 2 ], var_data_ptr->data[ 3 ],
                    var_data_ptr->data[ 4 ], var_data_ptr->data[ 5 ] );
            imx_set_sensor( imx_get_wifi_bssid_scb(), (void *) var_data_ptr );
        } else
            imx_printf( "Unable to get variable length data buffer\r\n" );
    }

    if( device_config.log_wifi_rssi == true ) {
        rssi = hal_get_wifi_rssi();
        imx_printf( " RSSI: %ld", rssi );
        imx_set_sensor( imx_get_wifi_rssi_scb(), (void *) &rssi );
    }

    if( device_config.log_wifi_rfnoise == true ) {
        noise = hal_get_wifi_noise();
        imx_printf( " RF Noise: %ld", noise );
        imx_set_sensor( imx_get_wifi_rf_noise_scb(), (void *) &noise );
    }

    imx_printf( "\r\n" );

}
