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
#include "../cs_ctrl/imx_cs_interface.h"
#include "../device/icb_def.h"
#include "../device/hal_wifi.h"


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
    wiced_mac_t bssid;

    if( device_config.log_wifi_AP == true ) {
        channel = hal_get_wifi_channel();
        hal_get_wifi_bssid( &bssid );
        imx_set_sensor( imx_get_wifi_bssid_scb(), (void *) &bssid );
        imx_set_sensor( imx_get_wifi_channel_scb(), &channel );
    }

    if( device_config.log_wifi_rssi == true ) {
        rssi = hal_get_wifi_rssi();
        imx_set_sensor( imx_get_wifi_rssi_scb(), (void *) &rssi );
    }

    if( device_config.log_wifi_rfnoise == true ) {
        noise = hal_get_wifi_noise();
        imx_set_sensor( imx_get_wifi_rf_noise_scb(), (void *) &noise );
    }


}
