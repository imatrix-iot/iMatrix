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

/** @file location.h
 *
 *  Created on: March 3, 2017
 *      Author: greg.phillips
 */

#ifndef LOCATAION_H_
#define LOCATAION__H_


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
 *               Function Definitions
 ******************************************************/
void init_location_system(void);
void process_location( wiced_time_t current_time );
float imx_get_longitude(void);
void imx_set_longitude(float longitude);
float imx_get_latitude(void);
void imx_set_latitude(float latitude);
float imx_get_elevation(void);
void imx_set_elevation(float elevation);
uint32_t imx_get_indoor_x(void);
void imx_set_indoor_x(uint32_t indoor_x);
uint32_t imx_get_indoor_y(void);
void imx_set_indoor_y(uint32_t indoor_y);
uint32_t imx_get_indoor_z(void);
void imx_set_indoor_z(uint32_t indoor_z);
uint32_t imx_get_building_id(void);
void imx_set_building_id(uint32_t building_id );
uint32_t imx_get_floor_id(void);
void imx_set_floor_id(uint32_t floor_id );
uint32_t imx_get_room_id(void);
void imx_set_room_id(uint32_t room_id );
uint32_t imx_get_group_id(void);
void imx_set_group_id(uint32_t group_id );
void imx_set_all_location( uint32_t local_seconds_offset_from_utc, float longitude, float latitude, float elevation, uint32_t indoor_x, uint32_t indoor_y, uint32_t indoor_z,
        uint32_t building_id, uint32_t floor_id, uint32_t room_id, uint32_t group_id );
#endif /* LOCATAION__H_ */
