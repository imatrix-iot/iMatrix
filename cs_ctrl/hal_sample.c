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
/** @file hal_sample.c
 *
 *  Created on: Feb 2, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../time/ck_time.h"
#include "hal_sample.h"
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
bool check_int_percent( int32_t current_value, int32_t last_value, uint16_t percentage );
bool check_uint_percent( uint32_t current_value, uint32_t last_value, uint16_t percentage );
bool check_float_percent( float current_value, float last_value, uint16_t percentage );

/******************************************************
 *               Variable Definitions
 ******************************************************/
uint16_t active_sensor = 0;
uint16_t active_control = 0;
extern IOT_Device_Config_t device_config;	// Defined in device\config.h
extern control_sensor_data_t cd[];
extern control_sensor_data_t sd[];
extern functions_t imx_control_functions[];
extern functions_t imx_sensor_functions[];
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief process a the sampling of controls and sensors
  * @param  None
  * @retval : None
  */
void hal_sample( peripheral_type_t type, wiced_time_t current_time )
{
	uint16_t i, *active;
	uint8_t status;
	bool percent_change_detected;
	control_sensor_data_t *data;
	control_sensor_block_t *csb;
	functions_t *f;

	if( type == IMX_CONTROLS ) {
		if( device_config.no_controls == 0 )
			return;	// Nothing to do
		else {
			active = &active_control;
			if( *active >= device_config.no_controls )
				*active = 0;
			data = &cd[ *active ];
			f = &imx_control_functions[ *active ];
			csb = &device_config.ccb[ *active ];
//			print_status( "Sampling Control: %u\r\n", *active );
		}
	} else {
		if( device_config.no_sensors == 0 )
			return;	// Nothing to do
		else {
			active = &active_sensor;
			if( *active >= device_config.no_sensors )
				*active = 0;
			data = &sd[ *active ];
//			print_status( "Sample - Setting Sensor %u Data to: 0x%08lx\r\n", *active, (uint32_t) data );
			f = &imx_sensor_functions[ *active ];
			csb = &device_config.scb[ *active ];
//			cli_print( "Sampling Sensor: %u\r\n", *active );
		}
	}
//    wiced_rtos_delay_milliseconds( 100 );
	/*
	 * Check Control / Sensor, update this sensor stored data if it changes warning level or sample rate is due
	 *
	 * Sample rate of 0 represents event driven
	 */
	if( ( csb->enabled == true ) && ( csb->sample_rate > 0 ) ) {
		status = 0;	// Controls may not have an update function as the may just be set remotely
		if( f->update != NULL ) {
			status = ( f->update)( f->arg, &data->data[ data->no_samples ] );
			csb->valid = true;  // We have a sample
			/*
			if( type == CONTROLS )
				print_status( "Sampled Control: %u, result: %u", *active, status );
			else
				print_status( "Sampled Sensor: %u, result: %u", *active, status );
			print_status( ", Value: " );
			switch( csb->data_type ) {
				case DI_INT32 :
					print_status( "%ld", data->data[ data->no_samples ].int_32bit );
					break;
				case AI_FLOAT :
					print_status( "%f", data->data[ data->no_samples ].float_32bit );
					break;
				case DI_UINT32 :
				default :
					print_status( "%lu", data->data[ data->no_samples ].uint_32bit );
					break;
			}
			print_status( "\r\n" );
			*/
		}
		if( status == 0 ) {
			data->error = status;	// Reset for correction
			/*
			 * Check if the data is in warning levels for the sensor
			 */
			data->warning = IMX_INFORMATIONAL;	// Assume for now
			data->last_value.uint_32bit = data->data[ data->no_samples ].uint_32bit;	// save value for reference
			/*
			 * Each time thru the loop will check for the next most severe level and set the highest by the end
			 */
			for( i = IMX_WATCH; i < IMX_WARNING_LEVELS; i++ ) {
				/*
				 * Do we check if below current level
				 */
				if( ( csb->use_warning_level_low & ( 0x1 << ( i - IMX_WATCH ) ) ) != 0 )
					switch( csb->data_type ) {
						case IMX_DI_INT32 :
							if( data->data[ data->no_samples ].int_32bit < csb->warning_level_low[ i - IMX_WATCH ].int_32bit )
								data->warning = i;	// Now set to this level
							break;
						case IMX_AI_FLOAT :
							if( data->data[ data->no_samples ].float_32bit < csb->warning_level_low[ i - IMX_WATCH ].float_32bit )
								data->warning = i;	// Now set to this level
							break;
						case IMX_DI_UINT32 :
						default :
							if( data->data[ data->no_samples ].uint_32bit < csb->warning_level_low[ i - IMX_WATCH ].uint_32bit )
								data->warning = i;	// Now set to this level
							break;
					}
				/*
				 * Do we check if above current level
				 */
				if( ( csb->use_warning_level_high & ( 0x1 << ( i - IMX_WATCH ) ) ) != 0 )
					switch( csb->data_type ) {
						case IMX_DI_INT32 :
							if( data->data[ data->no_samples ].int_32bit > csb->warning_level_low[ i - IMX_WATCH ].int_32bit )
								data->warning = i;	// Now set to this level
							break;
						case IMX_AI_FLOAT :
							if( data->data[ data->no_samples ].float_32bit > csb->warning_level_low[ i - IMX_WATCH ].float_32bit )
								data->warning = i;	// Now set to this level
							break;
						case IMX_DI_UINT32 :
						default :
							if( data->data[ data->no_samples ].uint_32bit > csb->warning_level_low[ i - IMX_WATCH ].uint_32bit )
								data->warning = i;	// Now set to this level
							break;
					}
			}
			/*
			 * Do we check percent change
			 */
			percent_change_detected = false;
			if( csb->send_on_percent_change == true )
				switch( csb->data_type ) {
					case IMX_DI_INT32 :
						if( check_int_percent( data->data[ data->no_samples ].int_32bit, data->last_value.int_32bit, csb->percent_change_to_send ) )
							percent_change_detected = true;
						break;
					case IMX_AI_FLOAT :
						if( check_float_percent( data->data[ data->no_samples ].float_32bit, data->last_value.float_32bit, csb->percent_change_to_send ) )
							percent_change_detected = true;
						break;
					case IMX_DI_UINT32 :
					default :
						if( check_uint_percent( data->data[ data->no_samples ].uint_32bit, data->last_value.uint_32bit, csb->percent_change_to_send ) )
							percent_change_detected = true;
						break;
				}

			/*
			 * See if we save this entry
			 *
			 * The criteria is based on the following
			 *
			 * is this sample on the sample rate so save as standard
			 * is this sample a change in warning level
			 * is this sample a change of >= Change percentage level - if enabled
			 *
			 */
			if( ( is_later( current_time, data->last_sample_time + (wiced_time_t) ( csb->sample_rate ) ) == true ) ||
				( data->warning != data->last_warning ) ||
				( percent_change_detected == true ) ) {

				// print_status( "Saving %s value for sensor: %u - %s - Saved entries: %u\r\n",
				//		type == CONTROLS ? "Control" : "Sensor", *active, device_config.scb[ *active ].name, ( data->no_samples + 1 ) );

				/*
				 * Check for overflow - Save only the last sample values
				 */
				if( data->no_samples >= ( HISTORY_SIZE - 1 ) ) {
					memmove( &data->data[ 0 ], &data->data[ 1 ], ( HISTORY_SIZE - 1 ) * SAMPLE_LENGTH );
				} else
					data->no_samples += 1;
				data->last_sample_time = current_time;
				/*
				 * See if the batch is ready to go
				 */
				if( ( data->warning != data->last_warning ) ||
					( data->no_samples >= device_config.scb[ *active ].sample_batch_size ) ||
					( data->no_samples >= ( HISTORY_SIZE - 1  ) ) || // We can't get any more in to this record
					( data->update_now == true ) ||
					( percent_change_detected == true ) ) {
					/*
					print_status( "Setting %s: %u, ID: 0x%08lx to send batch of: %u, batch size %u, sample_now: %s sensor_warning: %u, last: %u, %%change detected: %s\r\n", type == IMX_CONTROLS ? "Control" : "Sensor",
							*active, type == IMX_CONTROLS ? device_config.ccb[ *active ].id : device_config.scb[ *active ].id, data->no_samples, csb->sample_batch_size, data->update_now ? "true" : "false",
							data->warning, data->last_warning, percent_change_detected ? "true" : "false" );
					*/
					data->update_now = false;
					data->last_warning = data->warning;
					data->send_batch = true;	// Send this now
				}
			}
		} else {
//			print_status( "Error Reading sensor %u\r\n", *active );
			data->errors += 1;
			data->error = status;
			/*
			 * see if change in error or all we are getting is errors. - Only send once per batch
			 */
			if( ( data->error != data->last_error ) ||
				( is_later( current_time, data->last_sample_time + (wiced_time_t) ( (uint32_t) device_config.scb[ *active ].sample_batch_size * 1000L  ) ) == true ) ) {
//				print_status( "Error: %u, Last Error: %u, current_time: %lu, time difference: %lu\r\n", data->error, data->last_error, data->last_sample_time, ( data->last_sample_time + (wiced_time_t) ( (uint32_t) device_config.scb[ *active ].sample_batch_size * 1000L  ) - (uint32_t) current_time )  );
				data->last_sample_time = current_time;
				data->last_error = data->error;
				data->send_on_error = true;
			}
		}
	}

	*active += 1;	// Process next sensor
}

/**
  * @brief	Check if the new value exceeds the previous by great than a defined percentage
  * 		Three routines, one for each type of data we support
  * @param  current value, last value, percentage
  * @retval : bool
  */
bool check_int_percent( int32_t current_value, int32_t last_value, uint16_t percentage )
{
	current_value = abs( current_value );
	last_value = abs( last_value );
//	print_status( "Comparing %ld against %ld (high) %ld (low)\r\n", ( current_value * 100 ), ( last_value * ( 100 + percentage ) ), ( last_value * ( 100 - percentage ) ) );

	if( ( current_value * 100 ) > ( last_value * ( 100 + percentage ) ) )
		return true;
	if( ( current_value * 100 ) < ( last_value * ( 100 - percentage ) ) )
		return true;
//	print_status( "With in range\r\n" );
	return false;
}
bool check_uint_percent( uint32_t current_value, uint32_t last_value, uint16_t percentage )
{
	if( ( current_value * 100 ) > ( last_value * ( 100 + percentage ) ) )
		return true;
	if( ( current_value * 100 ) < ( last_value * ( 100 - percentage ) ) )
		return true;
	return false;
}
bool check_float_percent( float current_value, float last_value, uint16_t percentage )
{
	current_value = (float) fabs( (double) current_value );
	last_value = (float) fabs( (double) last_value );
//	print_status( "Comparing %f against %f (high) %f (low)\r\n", ( current_value ), ( last_value * ( 1 + (float)percentage/100 ) ), ( last_value * ( 1 - (float)percentage/100 ) ) );
	if( ( current_value ) > ( last_value * ( 1 + ( (float)percentage/100 ) ) ) )
		return true;
	if( ( current_value ) < ( last_value * ( 1 - ( (float)percentage/100 ) ) ) )
		return true;
//	print_status( "With in range\r\n" );
	return false;
}
