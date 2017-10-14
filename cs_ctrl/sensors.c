/*
 * $Copyright 2013-2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file sensors.c
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */

#include <stdint.h>
#include <stdbool.h>
#include <wiced.h>

#include "../imatrix.h"
#include "../storage.h"
#include "../cli/interface.h"
#include "sensors.h"
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

extern control_sensor_data_t sd[ MAX_NO_SENSORS ];
extern control_sensor_block_t sensors_defaults[];
extern functions_t sensor_functions[];
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	reset the sensor system
  *
  * Add all sensors on our own platform that are enabled and any that are allocated from the Arduino device
  * @param  None
  * @retval : None
  */
void reset_sensors(void)
{
	uint16_t i, scb_entry, count;

	count = device_config.no_sensors;
	print_status( "Adding %u Sensors\r\n", count );
	for( i = 0; i < count; i++ ) {
/*        printf( "Loading defaults from Sensor Defaults: %u, %s, id: %u, sample rate: %u, batch size: %u, percent change: %u, enabled: %u, send on percent: %u, data type: %u\r\n", i,
                sensors_defaults[ i ].name,
                (uint16_t) sensors_defaults[ i ].id,
                (uint16_t) sensors_defaults[ i ].sample_rate,
                (uint16_t) sensors_defaults[ i ].sample_batch_size,
                (uint16_t) sensors_defaults[ i ].percent_change_to_send,
                (uint16_t) sensors_defaults[ i ].enabled,
                (uint16_t) sensors_defaults[ i ].send_on_percent_change,
                (uint16_t) sensors_defaults[ i ].data_type );
*/
		add_sensor( &sensors_defaults[ i ], &scb_entry );

	}
}

/**
  * @brief	add a new sensor to the system
  * @param  pointer to sensor block
  * @retval : Error code
  */
uint16_t add_sensor( control_sensor_block_t *ctrl_blk, uint16_t *entry_no )
{

	if(device_config.no_sensors >= MAX_NO_SENSORS )
		return IMX_MAXIMUM_SENSORS_EXCEEDED;

	/*
	 * Setup call back entries.
	 */
	memcpy( &device_config.scb[ device_config.no_sensors ], ctrl_blk, sizeof( control_sensor_block_t ) );
	*entry_no = device_config.no_sensors;

	print_status( "Added new sensor: %u%s, %s ", device_config.no_sensors, device_config.scb[ device_config.no_sensors ].enabled ? "" : " (Disabled)", device_config.scb[ device_config.no_sensors ].name );

    if( device_config.scb[ device_config.no_sensors ].sample_rate == 0 )
        print_status( "Event Driven" );
    else
        print_status( "Sample Rate Every: %u Sec", device_config.scb[ device_config.no_sensors ].sample_rate );
    print_status( " Batch size: %u\r\n", device_config.scb[ device_config.no_sensors ].sample_batch_size );

	device_config.no_sensors += 1;

	return IMX_SUCCESS;
}

/**
  * @brief	Initailize Sensor Collection System
  * @param  None
  * @retval : None
  */
void init_sensors(void)
{
	uint16_t i;
	wiced_time_t current_time;

    /*
     * This data is in CCMSRAM - Initialize to zeros
     */
    memset( &sd, 0x00, sizeof( sd ) );

	/*
	 * Set last sample time as now, +1 second for each sensor
	 */
	wiced_time_get_time( &current_time );	// Current time in mS

	for( i = 0; i < device_config.no_sensors; i++ ) {
		if( sensor_functions[ i ].init != NULL )
			(sensor_functions[ i ].init)( sensor_functions[ i ].arg  );	// Initialize sensor
		sd[ i ].update_now = true;		// Sample first time
		sd[ i ].last_sample_time = current_time + ( i * 1000 );
		sd[ i ].no_samples = 0;
		sd[ i ].errors = 0;
		sd[ i ].last_warning = IMX_INFORMATIONAL;
	}
}
void load_config_defaults_generic_scb( uint16_t arg )
{
	memcpy( &device_config.scb[ arg ], &sensors_defaults[ arg ], sizeof( control_sensor_block_t ) );
}

void print_sensors(void)
{
	uint16_t i;

    cli_print( "Sensors - sd @: 0x%08lx\r\n", (uint32_t) &sd );

	for( i = 0; i < device_config.no_sensors; i++ ) {
		cli_print( "%s, 0x%08lx, last value: ", device_config.scb[ i ].name, device_config.scb[ i ].id );
		switch( device_config.scb[ i ].data_type ) {
			case IMX_DI_UINT32 :
				cli_print( "%lu ", sd[ i ].last_value.uint_32bit );
				break;
			case IMX_DI_INT32 :
				cli_print( "%ld ", sd[ i ].last_value.int_32bit );
				break;
			case IMX_AI_FLOAT :
				cli_print( "%f ", sd[ i ].last_value.float_32bit );
				break;
		}
		cli_print( "\r\n" );
	}

}
