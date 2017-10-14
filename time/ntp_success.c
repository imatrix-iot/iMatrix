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

#include "wiced.h"

#include "../system.h"
#include "../defines.h"
#include "../device/dcb_def.h"
#include "../device/config.h"
#include "../hal_support.h"
#include "ntp_success.h"

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
extern dcb_t dcb;
extern IOT_Device_Config_t device_config;
uint32_t ntp_succeeded_since_boot = WICED_FALSE;
uint32_t save_time_to_sflash = WICED_FALSE;// To avoid multithreading problems use a 32 bit number.
                                           // Thus a single machine instruction is all that is required to store this value.

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	Flag that we got a valid NTP time
  * @param  None
  * @retval : None
  */
void ntp_success(void)
{
    wiced_utc_time_t utc_time;
    // uint32_t local_time;
    ntp_succeeded_since_boot = WICED_TRUE;
    save_time_to_sflash = WICED_TRUE;
    dcb.time_set_with_NTP = true;
    /*
     * Set the time for AT Host devices
     */
    wiced_time_get_utc_time( &utc_time );
    // local_time = (uint32_t) utc_time + device_config.local_seconds_offset_from_utc;
    //set_AT_rtc( local_time );

}
