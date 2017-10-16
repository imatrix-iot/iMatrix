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


#include "../device/icb_def.h"
#include "../device/hal_wifi.h"


/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_HISTORY
    #undef PRINTF
    #define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_HISTORY ) != 0x00 ) st_log_print_status(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

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

extern dcb_t dcb;
extern history_data_t history;
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
  * @brief  measure and store the RSSI and Noise levels for Wi Fi
  *
  * @param  None
  * @retval : completion
  */
void wifi_logging(void)
{
	int32_t rssi, noise;

	if( dcb.wifi_up ) {
		rssi = hal_get_wifi_rssi();
		noise = hal_get_wifi_noise();
	} else {
		rssi = 0;
		noise = 0;
	}
    memmove( &history.data[ HISTORY_WIFI_RSSI ][ 1 ], &history.data[ HISTORY_WIFI_RSSI ][ 0 ], ( HISTORY_SIZE - 1 ) * sizeof( int16_t ) );
    history.sensor_data_valid[ HISTORY_WIFI_RSSI ] <<= 1;

    history.data[ HISTORY_WIFI_RSSI ][ 0 ] = (int16_t) rssi;
    history.sensor_data_valid[ HISTORY_WIFI_RSSI ] |= 0x01;
    PRINTF( "Saving new WiFi RSSI value %d dB\r\n", history.data[ HISTORY_WIFI_RSSI ][ 0 ] );

    memmove( &history.data[ HISTORY_WIFI_NOISE ][ 1 ], &history.data[ HISTORY_WIFI_NOISE ][ 0 ], ( HISTORY_SIZE - 1 ) * sizeof( int16_t ) );
    history.sensor_data_valid[ HISTORY_WIFI_NOISE ] <<= 1;

    history.data[ HISTORY_WIFI_NOISE ][ 0 ] = (int16_t) noise;
    history.sensor_data_valid[ HISTORY_WIFI_NOISE ] |= 0x01;
    PRINTF( "Saving new WiFi Noise value %0.3f Watts\r\n", (float ) history.data[ HISTORY_WIFI_NOISE ][ 0 ] );
}

