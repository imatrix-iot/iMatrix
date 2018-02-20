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
#include "../storage.h"
#include "../device/config.h"
#include "../device/icb_def.h"
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
	float latitude;
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
extern iMatrix_Control_Block_t icb;

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
	if( ( device_config.latitude == 0 ) && ( device_config.longitude == 0 ) )
		return;

	wait_for_update = 0;
	last_gps_update = 0;
	gps_index = 0;
	icb.send_gps_coords = true;
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
		icb.latitude = simple_gps_route[ gps_index ].latitude;
		icb.longitude = simple_gps_route[ gps_index ].longitude;
        gps_index += 1;
        if( gps_index >= NO_GPS_ENTRIES )
            gps_index = 0;
	}
	if( is_later( current_time, last_gps_update + device_config.location_update_rate ) == true )
		icb.send_gps_coords = true;
#endif
}
/**
  * @brief Get the longitude of the Thing
  * @param  None
  * @retval : longitude
  */
float imx_get_longitude(void)
{
    return icb.longitude;
}
/**
  * @brief Set the longitude of the Thing
  * @param  None
  * @retval : longitude
  */
void imx_set_longitude(float longitude)
{
    icb.longitude = longitude;
}
/**
  * @brief Get the latitude of the Thing
  * @param  None
  * @retval : latitude
  */
float imx_get_latitude(void)
{
    return icb.latitude;
}
/**
  * @brief Set the latitude of the Thing
  * @param  None
  * @retval : imx_get_latitude
  */
void imx_set_latitude(float latitude)
{
    icb.latitude = latitude;
}
/**
  * @brief Get the elevation of the Thing
  * @param  None
  * @retval : elevation
  */
float imx_get_elevation(void)
{
    return icb.elevation;
}
/**
  * @brief Set the elevation of the Thing
  * @param  None
  * @retval : imx_get_elevation
  */
void imx_set_elevation(float elevation)
{
    icb.elevation = elevation;
}
/**
  * @brief Get the indoor_x of the Thing
  * @param  None
  * @retval : indoor_x
  */
uint32_t imx_get_indoor_x(void)
{
    return icb.indoor_x;
}
/**
  * @brief Set the indoor_x of the Thing
  * @param  indoor_x
  * @retval : None
  */
void imx_set_indoor_x(uint32_t indoor_x)
{
    icb.indoor_x = indoor_x;
}
/**
  * @brief Get the indoor_y of the Thing
  * @param  None
  * @retval : indoor_y
  */
uint32_t imx_get_indoor_y(void)
{
    return icb.indoor_y;
}
/**
  * @brief Set the indoor_y of the Thing
  * @param  indoor_y
  * @retval : None
  */
void imx_set_indoor_y(uint32_t indoor_y)
{
    icb.indoor_y = indoor_y;
}
/**
  * @brief Get the indoor_z of the Thing
  * @param  None
  * @retval : indoor_z
  */
uint32_t imx_get_indoor_z(void)
{
    return icb.indoor_z;
}
/**
  * @brief Set the indoor_z of the Thing
  * @param  indoor_z
  * @retval : None
  */
void imx_set_indoor_z(uint32_t indoor_z)
{
    icb.indoor_z = indoor_z;
}
/**
  * @brief Get the building ID of the Thing
  * @param  None
  * @retval : building ID
  */
uint32_t imx_get_building_id(void)
{
    return device_config.building_id;
}
/**
  * @brief Set the building ID of the Thing
  * @param  building id
  * @retval : None
  */
void imx_set_building_id(uint32_t building_id )
{
    device_config.building_id = building_id;
    imatrix_save_config();
}
/**
  * @brief Get the floor ID of the Thing
  * @param  None
  * @retval : floor ID
  */
uint32_t imx_get_floor_id(void)
{
    return device_config.floor_id;
}
/**
  * @brief Set the floor ID of the Thing
  * @param  floor id
  * @retval : None
  */
void imx_set_floor_id(uint32_t floor_id )
{
    device_config.floor_id = floor_id;
    imatrix_save_config();
}
/**
  * @brief Get the room ID of the Thing
  * @param  None
  * @retval : Room ID
  */
uint32_t imx_get_room_id(void)
{
    return device_config.room_id;
}
/**
  * @brief Set the room ID of the Thing
  * @param  room id
  * @retval : None
  */
void imx_set_room_id(uint32_t room_id )
{
    device_config.room_id = room_id;
    imatrix_save_config();
}
/**
  * @brief Get the group ID of the Thing
  * @param  None
  * @retval : Group ID
  */
uint32_t imx_get_group_id(void)
{
    return device_config.group_id;
}
/**
  * @brief Set the group ID of the Thing
  * @param  group id
  * @retval : None
  */
void imx_set_group_id(uint32_t group_id )
{
    device_config.group_id = group_id;
    imatrix_save_config();
}
