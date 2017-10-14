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
 * keep_alive.c
 *
 *  Created on: Oct 10, 2016
 *      Author: Eric Thelin
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Null function Data Frame keep alive parameters */
#define KEEP_ALIVE_ID_NFD          0

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Start sending Null function keep alive messages to the AP every "wait_milliseconds_for_every_send".
 * If "wait_milliseconds_for_every_send" == 0, no messages will be sent or a previously
 * configured keep alive will be stopped.
 * Returns the WWD error code typecast into wiced_result_t.
 *
 * written by Eric Thelin 13 October 2016
 */
wiced_result_t start_keep_alive( uint32_t wait_milliseconds_for_every_send )
{
    static wiced_keep_alive_packet_t keep_alive_packet_info;

    /* Setup a Null function data frame keep alive */
    keep_alive_packet_info.keep_alive_id = KEEP_ALIVE_ID_NFD;
    keep_alive_packet_info.period_msec   = wait_milliseconds_for_every_send;
    keep_alive_packet_info.packet_length = 0;
    keep_alive_packet_info.packet = NULL;

    return wiced_wifi_add_keep_alive( &keep_alive_packet_info );
}

/**
 * Stop sending Null function keep alive messages to the AP.
 * Returns the WWD error code typecast into wiced_result_t.
 *
 * written by Eric Thelin 13 October 2016
 */
wiced_result_t stop_keep_alive()
{
	// All the following function does is re-add the keep alive with a period of 0.

    return wiced_wifi_disable_keep_alive( KEEP_ALIVE_ID_NFD );
}
