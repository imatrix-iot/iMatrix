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
#include "../cli/messages.h"
#include "../device/var_data.h"
#include "hal_sample.h"
#include "hal_event.h"
#include "imx_cs_interface.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_EVENTS_DRIVEN
    #undef PRINTF
    #define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_EVENTS_DRIVEN ) != 0x00 ) imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

/*
 *  Set up standard variables based on the type of data we are using
 */
#define SET_CSB_VARS( type )                        \
                if( type == IMX_CONTROLS ) {        \
                    csb = &device_config.ccb[ 0 ];  \
                    csd = &cd[ 0 ];                  \
                } else {                            \
                    csb = &device_config.scb[ 0 ];  \
                    csd = &sd[ 0 ];                  \
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
extern control_sensor_data_t *cd;
extern control_sensor_data_t *sd;
extern char *imx_data_types[ IMX_NO_DATA_TYPES ];
extern imx_functions_t imx_control_functions[];

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief set a sensor to a value - trigger and event if event driven
  * @param  sensor entry, value to set
  * @retval : result
  */
/*
 * Get & Set Control / Sensor data
 */
imx_status_t imx_get_control_sensor( imx_peripheral_type_t type, uint16_t entry, void *value )
{
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;    // Temp pointer to control structure

    SET_CSB_VARS( type );

    if( ( ( type == IMX_SENSORS ) && ( entry > device_config.no_sensors ) ) ||
        ( ( type == IMX_CONTROLS ) && ( entry > device_config.no_controls ) ) )
        return IMX_INVALID_ENTRY;

    if( csb[ entry ].enabled == false )
        return IMX_CONTROL_DISABLED;
    if( csd[ entry ].valid == true ) {
        memcpy( value, &csd[ entry ].last_value, SAMPLE_LENGTH );
        return IMX_SUCCESS;
    } else
        return IMX_NO_DATA;
}

imx_status_t imx_set_control_sensor( imx_peripheral_type_t type, uint16_t entry, void *value )
{
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;    // Temp pointer to control structure

    SET_CSB_VARS( type );

    PRINTF( "Saving %s: %u, Value Address 0x%08x\r\n", ( type == IMX_CONTROLS ) ? "Control" : "Sensor", entry, value );
    if( ( ( type == IMX_SENSORS ) && ( entry > device_config.no_sensors ) ) ||
        ( ( type == IMX_CONTROLS ) && ( entry > device_config.no_controls ) ) )
        return IMX_INVALID_ENTRY;

    if( csb[ entry ].enabled == false ) {
        if( type == IMX_CONTROLS )
            return IMX_CONTROL_DISABLED;
        else
            return IMX_SENSOR_DISABLED;
    }

    /*
     * If this is a variable length entry free up the save current to last value
     */
    if( csb[ entry ].data_type == IMX_VARIABLE_LENGTH ) {
        /*
         * Free up last entry if there was one
         */
//        print_var_pools();
        imx_data_32_t *foo;
        foo = (imx_data_32_t*) value;
        PRINTF( "*** Last value: 0x%08lx, new value @ 0x%08lx, length: %u\r\n", (uint32_t) csd[ entry ].last_value.var_data, (uint32_t) value, foo->var_data->length );
        if( csd[ entry ].last_value.var_data != NULL ) {
            PRINTF( "Freeing up existing variable length entry\r\n" );
            imx_add_var_free_pool( csd[ entry ].last_value.var_data );
        }
        PRINTF( "*** Getting variable length data for %s: %u, with from: 0x%08lx, length: %u\r\n",
                ( type == IMX_CONTROLS ) ? "Control" : "Sensor", entry, (uint32_t) value, ((imx_data_32_t *) value)->var_data->length );
        /*
         * Get a spare variable length entry to save the values in - should be available if same or < length as before
         */
        csd[ entry ].last_value.var_data = imx_get_var_data( ((imx_data_32_t *) value)->var_data->length );
        if( csd[ entry ].last_value.var_data != NULL ) {
            memcpy( (char *) csd[ entry ].last_value.var_data->data, (char *) ((imx_data_32_t *) value)->var_data->data, ((imx_data_32_t *) value)->var_data->length );
            csd[ entry ].last_value.var_data->length = ((imx_data_32_t *) value)->var_data->length;
        } else {
            imx_printf( "Unable to save variable length sensor data - Variable data pool empty\r\n" );
            return IMX_OUT_OF_MEMORY;
        }
//        print_var_pools();
    } else {
        /*
         * copy the value and do any action needed - Note this is for just raw uint, int and float data
         */
        PRINTF( "Copying %s: %u to last value @ 0x%08lx\r\n", ( type == IMX_CONTROLS ) ? "Control" : "Sensor", entry, &csd[ entry ].last_value );
        memcpy( &csd[ entry ].last_value, value, SAMPLE_LENGTH );
    }
    csd[ entry ].valid = true;  // We have a sample
    PRINTF( "%s: %u Now Valid\r\n", ( type == IMX_CONTROLS ) ? "Control" : "Sensor", entry );

    /*
     * If this is a control then we might need to do some action
     */
    if( type == IMX_CONTROLS ) {
        if( imx_control_functions[ entry ].update != NULL )
            (imx_control_functions[ entry ].update)( entry, value );
    }
    /*
     * Is this event driven - if so process the event into history
     */
    if( csb[ entry ].sample_rate == 0 ) {
        hal_event( type, entry, value );
    }

    return IMX_SUCCESS;
}
/**
  * @brief  parse the string to get the "value" of the argument based on the expected type
  * @param  pointer to provided string, pointer to result to pass to imx_x_set value
  * @retval : None
  */
bool imx_parse_value( imx_peripheral_type_t type, uint16_t entry, char *string, imx_data_32_t *value )
{
    uint16_t string_length;
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;    // Temp pointer to control structure

    SET_CSB_VARS( type );
    UNUSED_PARAMETER( csd );

//    imx_printf( "Processing assignment for %s( %s ): %u, data type(%s): %s with string value %s\r\n", type == IMX_CONTROLS ? "Control" : "Sensor",
//            csb[ entry ].name, entry, csb[ entry ].read_only == true ? "Read Only" : "Read/Write", imx_data_types[ csb[ entry ].data_type ], string );
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
            string_length = strlen( string ) + 1;
            value->var_data = imx_get_var_data( string_length );
            if( value->var_data != NULL ) {
                strcpy( (char *) value->var_data->data, string );
                value->var_data->length = string_length;
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
