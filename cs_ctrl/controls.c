/*
 * $Copyright 2013-2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file controls.c
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */

#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "../imatrix.h"
#include "storage.h"
#include "../cli/interface.h"
#include "controls.h"
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

extern control_sensor_data_t cd[ MAX_NO_CONTROLS ];
extern control_sensor_block_t imx_controls_defaults[];
extern functions_t imx_control_functions[];
extern IOT_Device_Config_t device_config;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief	reset the controls system
  *
  * Add all controls on our own platform that are enabled and any that are allocated from the Arduino device
  * @param:  None
  * @retval : None
  */
void reset_controls(void)
{
	uint16_t i, ccb_entry, count;

	count = device_config.no_controls;

	cli_print( "Controls Data @: 0x%08lx\r\n", (uint32_t ) &cd );
	cli_print( "Adding %u Controls\r\n", count );
	for( i = 0; i < count; i++ ) {
/*	    printf( "Loading defaults from Controls Defaults: %u, %s, id: %u, sample rate: %u, batch size: %u, percent change: %u, enabled: %u, send on percent: %u, data type: %u\r\n", i,
                imx_controls_defaults[ i ].name,
                (uint16_t) imx_controls_defaults[ i ].id,
                (uint16_t) imx_controls_defaults[ i ].sample_rate,
                (uint16_t) imx_controls_defaults[ i ].sample_batch_size,
                (uint16_t) imx_controls_defaults[ i ].percent_change_to_send,
                (uint16_t) imx_controls_defaults[ i ].enabled,
                (uint16_t) imx_controls_defaults[ i ].send_on_percent_change,
                (uint16_t) imx_controls_defaults[ i ].data_type );
*/
		add_control( &imx_controls_defaults[ i ], &ccb_entry );

	}
}
/**
 *  @brief	initialize the controls system
  * @param  None
  * @retval : None
  */
void init_controls(void)
{
	uint16_t i;
	wiced_time_t current_time;

	/*
	 * This data is in CCMSRAM - Initialize to zeros
	 */
	memset( &cd, 0x00, sizeof( cd ) );
	/*
	 * Set last control update time as now
	 */
	wiced_time_get_time( &current_time );	// Current time in mS

	for( i = 0; i < device_config.no_controls; i++ ) {
		if( imx_control_functions[ i ].init != NULL )
			(imx_control_functions[ i ].init)( imx_control_functions[ i ].arg );	// Initialize control
		cd[ i ].update_now = true;
		cd[ i ].last_sample_time = current_time + ( i * 1000 );
	}
}

/**
  * @brief	add a new control to the system
  * @param  pointer controls block
  * @retval : Error code
  */
uint16_t add_control( control_sensor_block_t *ctrl_blk, uint16_t *entry_no )
{

	if(device_config.no_controls >= MAX_NO_CONTROLS )
		return IMX_MAXIMUM_CONTROLS_EXCEEDED;

	/*
	 * Setup call back entries.
	 */
	memcpy( &device_config.ccb[ device_config.no_controls ], ctrl_blk, sizeof( control_sensor_block_t ) );
	*entry_no = device_config.no_controls;

	imx_printf( "Added new control: %u%s, %s, ", device_config.no_controls, device_config.ccb[ device_config.no_controls ].enabled ? "" : " (Disabled)", device_config.ccb[ device_config.no_controls ].name );

	if( device_config.ccb[ device_config.no_controls ].sample_rate == 0 )
	    imx_printf( "Event Driven" );
	else
	    imx_printf( "Sample Rate Every: %u Sec", device_config.ccb[ device_config.no_controls ].sample_rate );
	imx_printf( " Batch size: %u\r\n", device_config.ccb[ device_config.no_controls ].sample_batch_size );

	device_config.no_controls += 1;

	return IMX_SUCCESS;
}
/**
  * @brief	Load the defaults in the config for a generic device
  * @param  sensor no assigned to device
  * @retval : None
  */
void load_config_defaults_generic_ccb( uint16_t arg )
{
	memcpy( &device_config.ccb[ arg ], &imx_controls_defaults[ arg ], sizeof( control_sensor_block_t ) );
}
void print_controls(void)
{
	uint16_t i;

    cli_print( "Controls - cd @: 0x%08lx\r\n", (uint32_t) &cd );

	for( i = 0; i < device_config.no_controls; i++ ) {
		cli_print( "%s: ID: 0x%08lx, last value: ", device_config.ccb[ i ].name, device_config.ccb[ i ].id );
		switch( device_config.ccb[ i ].data_type ) {
			case IMX_DI_UINT32 :
				cli_print( "%lu ", cd[ i ].last_value.uint_32bit );
				break;
			case IMX_DI_INT32 :
				cli_print( "%ld ", cd[ i ].last_value.int_32bit );
				break;
			case IMX_AI_FLOAT :
				cli_print( "%f ", cd[ i ].last_value.float_32bit );
				break;
		}
		cli_print( "\r\n" );
	}

}
