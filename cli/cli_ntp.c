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

/** @file cli_ntp.c
 *
 *  Created on: November 25, 2017
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"
#include "interface.h"
#include "../time/ck_time.h"

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

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief resync NTP
  * @param  None
  * @retval : None
  */
void cli_ntp( uint16_t arg )
{
    UNUSED_PARAMETER( arg );
    wiced_iso8601_time_t iso8601_time;

    imx_printf( "\r\nNTP Request Start.....\r\n" );

    if( sntp_restart_auto_time_sync() == WICED_SUCCESS ) {

        wiced_time_get_iso8601_time( &iso8601_time );
        imx_printf("Current UTC time is: %.26s\n", (char*)&iso8601_time);
    } else
        imx_printf( "NTP Failed\r\n" );

}
