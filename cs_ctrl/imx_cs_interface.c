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
#include "../cli/interface.h"
#include "../device/var_data.h"
#include "hal_sample.h"
#include "hal_event.h"
#include "imx_cs_interface.h"
/******************************************************
 *                      Macros
 ******************************************************/
/*
 *  Set up standard variables based on the type of data we are using
 */
#define SET_CSB_VARS( type )                        \
                if( type == IMX_CONTROLS ) {        \
                    csb = &device_config.ccb[ 0 ];  \
                    csd = cd[ 0 ];                  \
                } else {                            \
                    csb = &device_config.scb[ 0 ];  \
                    csd = sd[ 0 ];                  \
                }

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
extern char *imx_data_types[ IMX_NO_DATA_TYPES ];
extern imx_functions_t imx_control_functions[], imx_sensor_functions[];

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
    data_32_t *foo;
    imx_printf( "Saving Sensor: %u, Value Address 0x%08x\r\n", entry, value );
    if( entry > device_config.no_sensors )
        return IMX_INVALID_ENTRY;
    if( device_config.scb[ entry ].enabled == false )
        return IMX_CONTROL_DISABLED;

    /*
     * If this is a variable length entry free up the save current to last value
     */
    if( device_config.scb[ entry ].data_type == IMX_VARIABLE_LENGTH ) {
        /*
         * Free up last entry if there was one
         */
        print_var_pools();
        foo = (data_32_t*) value;
        imx_printf( "*** Last value: 0x%08lx, value @ 0x%08lx, length: %u\r\n", (uint32_t) sd[ entry ]->last_value.var_data, (uint32_t) value, foo->var_data->header.length );
        if( sd[ entry ]->last_value.var_data != NULL ) {
            imx_printf( "Freeing up existing variable length entry\r\n" );
            imx_add_var_free_pool( sd[ entry ]->last_value.var_data );
        }
        imx_printf( "*** Getting variable length data for Sensor: %u, with from: 0x%08lx, length: %u\r\n", entry, (uint32_t) value, ((data_32_t *) value)->var_data->header.length );
        /*
         * Get a spare variable length entry to save the values in - should be available if same or < length as before
         */
        sd[ entry ]->last_value.var_data = imx_get_var_data( ((data_32_t *) value)->var_data->header.length );
        if( sd[ entry ]->last_value.var_data != NULL ) {
            memcpy( (char *) cd[ entry ]->last_value.var_data->data, (char *) ((data_32_t *) value)->var_data->data, ((data_32_t *) value)->var_data->header.length );
            sd[ entry ]->last_value.var_data->header.length = ((data_32_t *) value)->var_data->header.length;
        } else {
            imx_printf( "Unable to save variable length sensor data - Variable data pool empty\r\n" );
            return IMX_OUT_OF_MEMORY;
        }
        print_var_pools();
    } else {
        /*
         * copy the value and do any action needed - Note this is for just raw uint, int and float data
         */
        imx_printf( "Copying Sensor: %u to last value\r\n", entry );
        memcpy( &sd[ entry ]->last_value, value, SAMPLE_LENGTH );
    }
    sd[ entry ]->valid = true;  // We have a sample
    imx_printf( "Sensor: %u Now Valid\r\n", entry );

    if( device_config.scb[ entry ].sample_rate == 0 )
        hal_event( IMX_SENSORS, entry, value );

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

    /*
     * If this is a variable length entry free up the save current to last value
     */
    if( device_config.ccb[ entry ].data_type == IMX_VARIABLE_LENGTH ) {
        /*
         * Free up last entry if there was one
         */
        if( cd[ entry ]->last_value.var_data != NULL )
            imx_add_var_free_pool( cd[ entry ]->last_value.var_data );
        /*
         * Get a spare variable length entry to save the values in - should be available if same or < length as before
         */
        cd[ entry ]->last_value.var_data = imx_get_var_data( ((data_32_t *) value)->var_data->header.length );
        if( cd[ entry ]->last_value.var_data != NULL ) {
            strcpy( (char *) cd[ entry ]->last_value.var_data->data, (char *) ((data_32_t *) value)->var_data->data );
            cd[ entry ]->last_value.var_data->header.length = ((data_32_t *) value)->var_data->header.length;
        } else
            return IMX_OUT_OF_MEMORY;
    } else {
        /*
         * copy the value and do any action needed - Note this is for just raw uint, int and float data
         */
        memcpy( &cd[ entry ]->last_value, value, SAMPLE_LENGTH );
    }
    cd[ entry ]->valid = true;  // We have a sample

    if( imx_control_functions[ entry ].update != NULL )
        (imx_control_functions[ entry ].update)( entry, value );

    if( device_config.ccb[ entry ].sample_rate == 0 )
        hal_event( IMX_CONTROLS, entry, value );

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
/**
  * @brief  parse the string to get the "value" of the argument based on the expected type
  * @param  pointer to provided string, pointer to result to pass to imx_x_set value
  * @retval : None
  */
bool imx_parse_value( peripheral_type_t type, uint16_t entry, char *string, data_32_t *value )
{
    uint16_t string_length;
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;    // Temp pointer to control structure

    SET_CSB_VARS( type );
    UNUSED_PARAMETER( csd );

    imx_printf( "Processing assignment for %s( %s ): %u, data type(%s): %s with string value %s\r\n", type == IMX_CONTROLS ? "Control" : "Sensor",
            csb[ entry ].name, entry, csb[ entry ].read_only == true ? "Read Only" : "Read/Write", imx_data_types[ csb[ entry ].data_type ], string );
    /*
     * Do some error checking
     */
    if( csb[ entry ].read_only == true ) {
        imx_printf( "Read Only entry\r\n" );
        return false;
    }
    if( type == IMX_CONTROLS ) {
        if( entry >= device_config.no_controls ) {
            imx_printf( "Entry exceeds number of items\r\n" );
            return false;
        }
    } else {
        if( entry >= device_config.no_sensors ) {
            imx_printf( "Entry exceeds number of items\r\n" );
            return false;
        }
    }
    switch( csb[ entry ].data_type ) {
        case IMX_INT32 :
            value->int_32bit = (int32_t) atoi( string );
            break;
        case IMX_FLOAT :
            value->float_32bit = (float) atof( string );
            break;
        case IMX_VARIABLE_LENGTH :
            /*
             * Get a data variable length block for this item
             */
            string_length = strlen( string );
            value->var_data = imx_get_var_data( string_length );
            if( value->var_data != NULL ) {
                strcpy( (char *) value->var_data->data, string );
                value->var_data->header.length = string_length;
            } else
                return false;
            break;
        case IMX_UINT32 :
        default :
            value ->uint_32bit = (uint32_t) atol( string );
            break;
    }
    return true;
}
