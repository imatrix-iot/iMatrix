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

/** @file .c
 *
 *  Created on: April 9, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../imatrix.h"
#include "../storage.h"
#include "hal_event.h"
#include "../cli/interface.h"
#include "../device/config.h"

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
extern IOT_Device_Config_t device_config;
extern control_sensor_data_t cd[ MAX_NO_CONTROLS ];
extern control_sensor_data_t sd[ MAX_NO_SENSORS ];

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	set a register managed by AT commands
  * @param  None
  * @retval : None
  */
uint16_t set_register( peripheral_type_t type, uint16_t entry, char *value )
{
	if( type == IMX_CONTROLS ) {
		if( ( entry >= device_config.no_controls ) || ( device_config.ccb[ entry ].read_only == true ) ) {
			return false;
		}
		/*
		 * Save value to last value - this will be returned by sample routine
		 */
        cli_print( "Setting AT IC%u Control( %s ): %u, data type: %u with value %s\r\n", entry, device_config.ccb[ entry].name, entry, device_config.ccb[ entry].data_type, value );
		switch( device_config.ccb[ entry ].data_type ) {
			case IMX_INT32 :
				cd[ entry ].last_value.int_32bit = (int32_t) atoi( value );
				break;
			case IMX_FLOAT :
				cd[ entry ].last_value.float_32bit = (float) atof( value );
				break;
			case IMX_UINT32 :
			default :
				cd[ entry ].last_value.uint_32bit = (uint32_t) atol( value );
//                cli_print( "cd @: 0x%08lx, Value Set to: %lu\r\n", (uint32_t) &cd, cd[ AT_CONTROL_START + entry ].last_value.uint_32bit );
				break;
		}
		/*
		 * Do we notify the server about this or is this control just sampled?
		 */
		if( device_config.ccb[ entry].sample_rate == 0 ) {
            /*
             * We just set the value of control without a sample rate so send a notification of this event, as controls/sensors with a sample rate of 0 are not uploaded
             */
//            cli_print( "Event Notification: Writing Control (AT): %u, Value: uint32: %lu, int32: %ld, float: %f\r\n", AT_CONTROL_START + entry,
//                    cd[ entry].last_value.uint_32bit, cd[ entry].last_value.int_32bit, cd[ entry].last_value.float_32bit );
            hal_event( IMX_CONTROLS, entry, &cd[ entry].last_value.uint_32bit );

		}
	} else {
		if( ( entry >= device_config.no_sensors ) || ( device_config.scb[ entry ].read_only == true ) ) {
			return false;
		}
		/*
		 * Save value to last value - this will be returned by sample routine
		 */
        cli_print( "Setting AT Sensor: %u, data type: %u with value %s\r\n", entry, device_config.scb[ entry].data_type, value );
		switch( device_config.scb[ entry ].data_type ) {
			case IMX_INT32 :
				sd[ entry ].last_value.int_32bit = (int32_t) atoi( value );
//				cli_print( "Value Set to: %lu\r\n", sd[ AT_SENSOR_START + entry].last_value.int_32bit );
				break;
			case IMX_FLOAT :
				sd[ entry ].last_value.float_32bit = (float) atof( value );
				break;
			case IMX_UINT32 :
			default :
				sd[ entry ].last_value.uint_32bit = (uint32_t) atol( value );
				break;
		}
        /*
         * Do we notify the server about this or is this control just sampled?
         */
        if( device_config.scb[ entry].sample_rate == 0 ) {
            /*
             * We just set the value of control without a sample rate so send a notification of this event, as controls/sensors with a sample rate of 0 are not uploaded
             */
//            cli_print( "Event Notification: Writing Sensor(AT): %u, Value: uint32: %lu, int32: %ld, float: %f\r\n", entry,
//                    sd[ AT_SENSOR_START + entry].last_value.uint_32bit, sd[ entry].last_value.int_32bit, sd[ entry].last_value.float_32bit );
            hal_event( IMX_SENSORS, entry, &sd[ entry].last_value.uint_32bit );

        }
	}
	return true;
}
/**
  * @brief	print a register managed by AT commands
  * @param  None
  * @retval : None
  */

uint16_t print_register( peripheral_type_t type, uint16_t entry )
{
	if( type == IMX_CONTROLS ) {
		if( entry >= device_config.no_controls )
			return false;
		switch( device_config.ccb[ entry].data_type ) {
			case IMX_INT32 :
				cli_print( "%d", cd[ entry].last_value.int_32bit );
				break;
			case IMX_FLOAT :
				cli_print( "%f", cd[ entry].last_value.float_32bit );
				break;
			case IMX_UINT32 :
			default :
				cli_print( "%u", cd[ entry].last_value.uint_32bit );
				break;
		}
	} else {
		if( entry >= device_config.no_sensors )
			return false;
		switch( device_config.scb[ entry].data_type ) {
			case IMX_INT32 :
				cli_print( "%d", sd[ entry].last_value.int_32bit );
				break;
			case IMX_FLOAT :
				cli_print( "%f", sd[ entry].last_value.float_32bit );
				break;
			case IMX_UINT32 :
			default :
				cli_print( "%u", sd[ entry].last_value.uint_32bit );
				break;
		}

	}
	cli_print( "\r\n" );
	return true;
}
