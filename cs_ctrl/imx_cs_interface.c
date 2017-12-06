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

/** @file imx_interface.c
 *
 *  Created on: October 26, 2017
 *      Author: greg.phillips
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"
#include "../storage.h"
#include "hal_sample.h"
#include "hal_event.h"
#include "imx_cs_interface.h"
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
extern IOT_Device_Config_t device_config;   // Defined in device\config.h
extern control_sensor_data_t *cd[];
extern control_sensor_data_t *sd[];

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief set a sensor to a value - trigger and event if event driven
  * @param  sensor entry, value to set
  * @retval : result
  */
/*
 * Set & Get Control / Sensor data
 */
imx_status_t imx_set_sensor( uint16_t entry, void *value )
{
    imx_printf( "Saving Sensor: %u, Value Address 0x%08x\r\n", entry, value );
    if( entry > device_config.no_sensors )
        return IMX_INVALID_ENTRY;
    if( device_config.scb[ entry ].enabled == false )
        return IMX_CONTROL_DISABLED;
    if( device_config.scb[ entry ].sample_rate == 0 )
        hal_event( IMX_SENSORS, entry, value );
    else {
        /*
         * If this is a variable length entry - check if we need to free up previous entry
         */
        imx_printf( "Saving Sensor to location: 0x%08x\r\n", (uint32_t) &sd[ entry ]->last_value);
        memcpy( &sd[ entry ]->last_value, value, SAMPLE_LENGTH );
        imx_printf( "Saved Sensor: %u, New value 0x%08lx\r\n", entry, sd[ entry ]->last_value.uint_32bit );
    }
    return IMX_SUCCESS;
}
imx_status_t imx_get_sensor( uint16_t entry, void *value )
{
    if( entry > device_config.no_sensors )
        return IMX_INVALID_ENTRY;
    if( device_config.scb[ entry ].enabled == false )
        return IMX_SENSOR_DISABLED;
    if( sd[ entry ]->valid == true ) {
        memcpy( value, &sd[ entry ]->last_value, SAMPLE_LENGTH );
        return IMX_SUCCESS;
    } else
        return IMX_NO_DATA;
}
imx_status_t imx_set_control( uint16_t entry, void *value )
{
    if( entry > device_config.no_controls )
        return IMX_INVALID_ENTRY;
    if( device_config.ccb[ entry ].enabled == false )
        return IMX_CONTROL_DISABLED;
    if( device_config.ccb[ entry ].sample_rate == 0 )
        hal_event( IMX_CONTROLS, entry, value );
    else {
        /*
         * If this is a variable length entry - check if we need to free up previous entry
         */
        memcpy( &cd[ entry ]->last_value, value, SAMPLE_LENGTH );
    }
    return IMX_SUCCESS;
}
imx_status_t imx_get_control( uint16_t entry, void *value )
{
    if( entry > device_config.no_sensors )
        return IMX_INVALID_ENTRY;
    if( device_config.ccb[ entry ].enabled == false )
        return IMX_CONTROL_DISABLED;
    if( cd[ entry ]->valid == true ) {
        memcpy( value, &cd[ entry ]->last_value, SAMPLE_LENGTH );
        return IMX_SUCCESS;
    } else
        return IMX_NO_DATA;
}
