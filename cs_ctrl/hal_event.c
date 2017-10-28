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
#include "hal_event.h"
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
extern IOT_Device_Config_t device_config;	// Defined in device\config.h
extern control_sensor_data_t cd[];
extern control_sensor_data_t sd[];
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief process an event
  *        Events have sample rate of 0 so they are not processed in sampling of data. This is only stored when an event occurs.
  *        This function handles the storing and batching of events.
  *        Sets flags picked up by iMatrix upload monitors so that these events generate uploads
  * @param: Type of Event, entry and value
  * @retval : None
  */
void hal_event( peripheral_type_t type, uint16_t entry, void *value )
{
	uint16_t i;
	bool percent_change_detected;
	control_sensor_data_t *data;
	control_sensor_block_t *csb;
	wiced_time_t current_time;
	wiced_utc_time_t upload_utc_time;

	wiced_time_get_time( &current_time );

	if( type == IMX_CONTROLS ) {
		if( entry >= device_config.no_controls )    // reporting no valid device
			return;	// Nothing to do
		else {
			data = &cd[ entry ];
			csb = &device_config.ccb[ entry ];
//			imx_printf( "Event Control: %u\r\n", entry );
		}
	} else {
		if( entry >= device_config.no_sensors )
			return;	// Nothing to do
		else {
			data = &sd[ entry ];
//			imx_printf( "Sample - Setting Sensor %u Data to: 0x%08lx\r\n", entry, (uint32_t) data );
			csb = &device_config.scb[ entry ];
//			( "Event Sensor: %u\r\n", entry );
		}
	}
    /*
     * Check for overflow - Save only the last sample values
     */
    if( data->no_samples >= ( HISTORY_SIZE - 2 ) ) {
        imx_printf( "History Full - dropping last sample\r\n" );
        memmove( &data->data[ 0 ], &data->data[ 2 ], ( HISTORY_SIZE - 2 ) * SAMPLE_LENGTH );
        data->no_samples -= 2;
    }
	/*
	 * Event Driven saves Time Stamp & Value pair
	 */
    wiced_time_get_utc_time( &upload_utc_time );
    memcpy( &data->data[ data->no_samples ],  &upload_utc_time, SAMPLE_LENGTH );
    data->no_samples += 1;
	memcpy( &data->data[ data->no_samples ], value, SAMPLE_LENGTH );
    csb->valid = true;  // We have a sample

    /*
    if( type == CONTROLS )
        imx_printf( "Sampled Control: %u, result: %u", entry, status );
    else
        imx_printf( "Sampled Sensor: %u, result: %u", entry, status );
    imx_printf( ", Value: " );
    switch( csb->data_type ) {
        case DI_INT32 :
            imx_printf( "%ld", data->data[ data->no_samples ].int_32bit );
            break;
        case AI_FLOAT :
            imx_printf( "%f", data->data[ data->no_samples ].float_32bit );
            break;
        case DI_UINT32 :
        default :
            imx_printf( "%lu", data->data[ data->no_samples ].uint_32bit );
            break;
    }
    imx_printf( "\r\n" );
    */
    /*
     * Check if the data is in warning levels for the sensor
     */
    data->warning = IMX_INFORMATIONAL;  // Assume for now
    /*
     * Each time thru the loop will check for the next most severe level and set the highest by the end
     */
    for( i = IMX_WATCH; i < IMX_WARNING_LEVELS; i++ ) {
        /*
         * Do we check if below current level - Note Variable length data is ignored
         */
        if( ( csb->use_warning_level_low & ( 0x1 << ( i - IMX_WATCH ) ) ) != 0 )
            switch( csb->data_type ) {
                case IMX_DI_INT32 :
                    if( data->data[ data->no_samples ].int_32bit < csb->warning_level_low[ i - IMX_WATCH ].int_32bit )
                        data->warning = i;  // Now set to this level
                    break;
                case IMX_AI_FLOAT :
                    if( data->data[ data->no_samples ].float_32bit < csb->warning_level_low[ i - IMX_WATCH ].float_32bit )
                        data->warning = i;  // Now set to this level
                    break;
                case IMX_DI_VARIABLE_LENGTH :
                    break;
                case IMX_DI_UINT32 :
                default :
                    if( data->data[ data->no_samples ].uint_32bit < csb->warning_level_low[ i - IMX_WATCH ].uint_32bit )
                        data->warning = i;  // Now set to this level
                    break;
            }
        /*
         * Do we check if above current level - Note Variable length data is ignored
         */
        if( ( csb->use_warning_level_high & ( 0x1 << ( i - IMX_WATCH ) ) ) != 0 )
            switch( csb->data_type ) {
                case IMX_DI_INT32 :
                    if( data->data[ data->no_samples ].int_32bit > csb->warning_level_low[ i - IMX_WATCH ].int_32bit )
                        data->warning = i;  // Now set to this level
                    break;
                case IMX_AI_FLOAT :
                    if( data->data[ data->no_samples ].float_32bit > csb->warning_level_low[ i - IMX_WATCH ].float_32bit )
                        data->warning = i;  // Now set to this level
                    break;
                case IMX_DI_VARIABLE_LENGTH :
                    break;
                case IMX_DI_UINT32 :
                default :
                    if( data->data[ data->no_samples ].uint_32bit > csb->warning_level_low[ i - IMX_WATCH ].uint_32bit )
                        data->warning = i;  // Now set to this level
                    break;
            }
    }
    /*
     * Do we check percent change  - Note Variable length data is ignored
     */
    percent_change_detected = false;
    if( csb->send_on_percent_change == true ) {
        switch( csb->data_type ) {
            case IMX_DI_INT32 :
                if( check_int_percent( data->data[ data->no_samples ].int_32bit, data->last_value.int_32bit, csb->percent_change_to_send ) )
                    percent_change_detected = true;
                break;
            case IMX_AI_FLOAT :
                if( check_float_percent( data->data[ data->no_samples ].float_32bit, data->last_value.float_32bit, csb->percent_change_to_send ) )
                    percent_change_detected = true;
                break;
            case IMX_DI_VARIABLE_LENGTH :
                break;
            case IMX_DI_UINT32 :
            default :
                if( check_uint_percent( data->data[ data->no_samples ].uint_32bit, data->last_value.uint_32bit, csb->percent_change_to_send ) )
                    percent_change_detected = true;
                break;
        }
    }

    data->no_samples += 1;
    data->last_sample_time = current_time;
    if( csb->data_type == IMX_DI_VARIABLE_LENGTH ) {
        /*
         * Clear the current data as it has now been saved in the history queue - this pointer is used to determine if the entry needs to be freed. The upload process will free this entry
         */
        data->last_value.var_data = NULL;
    }
    imx_printf( "Event added\r\n" );
    /*
     * See if the batch is ready to go now
     */
    if( ( data->warning != data->last_warning ) ||
        ( ( data->no_samples / 2 ) >= device_config.scb[ entry ].sample_batch_size ) ||
        ( data->update_now == true ) ||
        ( percent_change_detected == true ) ) {

        imx_printf( "Setting %s: %u, ID: 0x%08lx to send batch of: %u, batch size %u, sample_now: %s sensor_warning: %u, last: %u, %%change detected: %s\r\n", type == IMX_CONTROLS ? "Control" : "Sensor",
                entry, type == IMX_CONTROLS ? device_config.ccb[ entry ].id : device_config.scb[ entry ].id, data->no_samples, csb->sample_batch_size, data->update_now ? "true" : "false",
                data->warning, data->last_warning, percent_change_detected ? "true" : "false" );

        data->update_now = false;
        data->last_warning = data->warning;
        data->send_batch = true;    // Send this now
    }
    /*
     * see if change in error or all we are getting is errors. - Only send once per batch
     */
    if( ( data->error != data->last_error ) ||
        ( is_later( current_time, data->last_sample_time + (wiced_time_t) ( (uint32_t) device_config.scb[ entry ].sample_batch_size * 1000L  ) ) == true ) ) {
    //              imx_printf( "Error: %u, Last Error: %u, current_time: %lu, time difference: %lu\r\n", data->error, data->last_error, data->last_sample_time, ( data->last_sample_time + (wiced_time_t) ( (uint32_t) device_config.scb[ entry ].sample_batch_size * 1000L  ) - (uint32_t) current_time )  );
        data->last_sample_time = current_time;
        data->last_error = data->error;
        data->send_on_error = true;
    }

}

