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

/** @file location.c
 *
 *  Created on: March 26, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../defines.h"
#include "../system.h"
#include "../hal.h"
#include "../system.h"
#include "../device/config.h"
#include "../device/dcb_def.h"
#include "../time/ck_time.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
//#define SIMULATE_GPS
#define NO_GPS_ENTRIES	28
/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct {
	float lattitude;
	float longitude;
	uint16_t delay;
} gps_update_entry_t ;
/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;
extern dcb_t dcb;

#ifdef SIMULATE_GPS
static gps_update_entry_t simple_gps_route[ NO_GPS_ENTRIES ] =
{
		{ 38.986847, -119.942922,5 },
		{ 38.986824, -119.943027,5 },
		{ 38.986860, -119.943139,5 },
		{ 38.986895, -119.943262,5 },
		{ 38.986858, -119.943489,5 },
		{ 38.986804, -119.943936,5 },
		{ 38.986689, -119.944121,5 },
		{ 38.986521, -119.944233,5 },
		{ 38.986304, -119.944385,5 },
		{ 38.986086, -119.944496,5 },
		{ 38.985899, -119.944508,5 },
		{ 38.985730, -119.944558,5 },
		{ 38.985535, -119.944723,5 },
		{ 38.985331, -119.944796,5 },
		{ 38.985125, -119.944527,5 },
		{ 38.984894, -119.944210,5 },
		{ 38.984674, -119.943890,5 },
		{ 38.984361, -119.943358,5 },
		{ 38.984031, -119.942908,60 },
		{ 38.984044, -119.942599,5 },
		{ 38.984410, -119.942167,5 },
		{ 38.984731, -119.941862,5 },
		{ 38.985254, -119.941681,5 },
		{ 38.985675, -119.941728,5 },
		{ 38.986191, -119.942011,5 },
		{ 38.986685, -119.942376,5 },
		{ 38.986915, -119.942680,5 },
		{ 38.986877, -119.942836,120 },

};
#endif
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief intialize location system
  * @param  None
  * @retval : None
  */
static wiced_time_t wait_for_update, last_gps_update;
static uint16_t gps_index;
void init_location_system(void)
{
	/*
	 * Currently system does not support serial GPS devices
	 *
	 * If the GPS location for this device is set - send it out at boot
	 */
	if( ( device_config.lattitude == 0 ) && ( device_config.longitude == 0 ) )
		return;

	wait_for_update = 0;
	last_gps_update = 0;
	gps_index = 0;
	dcb.send_gps_coords = true;
}
/**
  * @brief process the location system
  * @param  None
  * @retval : None
  */

void process_location( wiced_time_t current_time )
{
#ifdef SIMULATE_GPS

	if( is_later( current_time, wait_for_update + simple_gps_route[ gps_index ].delay ) == true ) {
		dcb.lattitude = simple_gps_route[ gps_index ].lattitude;
		dcb.longitude = simple_gps_route[ gps_index ].longitude;
        gps_index += 1;
        if( gps_index >= NO_GPS_ENTRIES )
            gps_index = 0;
	}
	if( is_later( current_time, last_gps_update + device_config.location_update_rate ) == true )
		dcb.send_gps_coords = true;
#endif
}
