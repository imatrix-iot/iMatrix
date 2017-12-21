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
#include "../cli/messages.h"
#include "../device/config.h"
#include "../device/var_data.h"
#include "../time/ck_time.h"
#include "hal_sample.h"
#include "hal_event.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_EVENTS_DRIVEN
    #undef PRINTF
    #define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_EVENTS_DRIVEN ) != 0x00 ) imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

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
extern IOT_Device_Config_t device_config;	// Defined in storage.h
extern control_sensor_data_t *cd;
extern control_sensor_data_t *sd;
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
void hal_event( imx_peripheral_type_t type, uint16_t entry, void *value )
{
	uint16_t i;
	bool percent_change_detected;
	control_sensor_data_t *csd;
	imx_control_sensor_block_t *csb;
	wiced_time_t current_time;
	wiced_utc_time_t upload_utc_time;

	wiced_time_get_time( &current_time );

	if( type == IMX_CONTROLS ) {
		if( entry >= device_config.no_controls )    // reporting no valid device
			return;	// Nothing to do
		else {
			csd = &cd[ 0 ];
			csb = &device_config.ccb[ 0 ];
		}
	} else {
		if( entry >= device_config.no_sensors )
			return;	// Nothing to do
		else {
			csd = &sd[ 0 ];
			csb = &device_config.scb[ 0 ];
		}
	}
	PRINTF( "Event - Setting %s %u Data @: 0x%08lx\r\n", type == IMX_CONTROLS ? "Control" : "Sensor", entry, (uint32_t) &csd[ entry ] );
    /*
     * Check for overflow - Save only the last sample values
     */
    if( csd[ entry ].no_samples >= ( device_config.history_size - 2 ) ) {
        PRINTF( "History Full - dropping last sample\r\n" );
        /*
         * If item is variable length - free before overwrite
         */
        if( csb[ entry ].data_type == IMX_VARIABLE_LENGTH )
            imx_add_var_free_pool( csd[ entry ].data[ 0 ].var_data );
        memmove( &csd[ entry ].data[ 0 ], &csd[ entry ].data[ 2 ], ( device_config.history_size - 2 ) * SAMPLE_LENGTH );
        csd[ entry ].no_samples -= 2;
    }
	/*
	 * Event Driven saves Time Stamp & Value pair
	 */
    wiced_time_get_utc_time( &upload_utc_time );
    memcpy( &csd[ entry ].data[ csd[ entry ].no_samples ],  &upload_utc_time, SAMPLE_LENGTH );
    csd[ entry ].no_samples += 1;
    /*
     * Add Data
     */

    if( csb[ entry ].data_type == IMX_VARIABLE_LENGTH ) {
        /*
         * Get a buffer and transfer this data to the entry
         */
        csd[ entry ].data[ csd[ entry ].no_samples ].var_data = imx_get_var_data( ((imx_data_32_t *) value)->var_data->length );
        if( csd[ entry ].data[ csd[ entry ].no_samples ].var_data == NULL ) {
            /*
             * No entry available, just drop this
             */
            return;
        }
        /*
         * Copy the data from the passed variable data
         */
        memcpy( csd[ entry ].data[ csd[ entry ].no_samples ].var_data->data, (char *) ((imx_data_32_t *) value)->var_data->data, ((imx_data_32_t *) value)->var_data->length );
        csd[ entry ].data[ csd[ entry ].no_samples ].var_data->length = ((imx_data_32_t *) value)->var_data->length;
    } else {
        /*
         * All Other Data is really just 32 bit
         */
        memcpy( &csd[ entry ].data[ csd[ entry ].no_samples ].uint_32bit, value, SAMPLE_LENGTH );
    }

    /*
     * Check if the data is in warning levels for the sensor
     */
    csd[ entry ].warning = IMX_INFORMATIONAL;  // Assume for now
    /*
     * Each time thru the loop will check for the next most severe level and set the highest by the end
     */
    for( i = IMX_WATCH; i < IMX_WARNING_LEVELS; i++ ) {
        /*
         * Do we check if below current level - Note Variable length data is ignored
         */
        if( ( csb[ entry ].use_warning_level_low & ( 0x1 << ( i - IMX_WATCH ) ) ) != 0 )
            switch( csb[ entry ].data_type ) {
                case IMX_INT32 :
                    if( csd[ entry ].data[ csd[ entry ].no_samples ].int_32bit < csb[ entry ].warning_level_low[ i - IMX_WATCH ].int_32bit )
                        csd[ entry ].warning = i;  // Now set to this level
                    break;
                case IMX_FLOAT :
                    if( csd[ entry ].data[ csd[ entry ].no_samples ].float_32bit < csb[ entry ].warning_level_low[ i - IMX_WATCH ].float_32bit )
                        csd[ entry ].warning = i;  // Now set to this level
                    break;
                case IMX_VARIABLE_LENGTH :
                    break;
                case IMX_UINT32 :
                default :
                    if( csd[ entry ].data[ csd[ entry ].no_samples ].uint_32bit < csb[ entry ].warning_level_low[ i - IMX_WATCH ].uint_32bit )
                        csd[ entry ].warning = i;  // Now set to this level
                    break;
            }
        /*
         * Do we check if above current level - Note Variable length data is ignored
         */
        if( ( csb[ entry ].use_warning_level_high & ( 0x1 << ( i - IMX_WATCH ) ) ) != 0 )
            switch( csb[ entry ].data_type ) {
                case IMX_INT32 :
                    if( csd[ entry ].data[ csd[ entry ].no_samples ].int_32bit > csb[ entry ].warning_level_low[ i - IMX_WATCH ].int_32bit )
                        csd[ entry ].warning = i;  // Now set to this level
                    break;
                case IMX_FLOAT :
                    if( csd[ entry ].data[ csd[ entry ].no_samples ].float_32bit > csb[ entry ].warning_level_low[ i - IMX_WATCH ].float_32bit )
                        csd[ entry ].warning = i;  // Now set to this level
                    break;
                case IMX_VARIABLE_LENGTH :
                    break;
                case IMX_UINT32 :
                default :
                    if( csd[ entry ].data[ csd[ entry ].no_samples ].uint_32bit > csb[ entry ].warning_level_low[ i - IMX_WATCH ].uint_32bit )
                        csd[ entry ].warning = i;  // Now set to this level
                    break;
            }
    }
    /*
     * Do we check percent change  - Note Variable length data is ignored
     */
    percent_change_detected = false;
    if( csb[ entry ].send_on_percent_change == true ) {
        switch( csb[ entry ].data_type ) {
            case IMX_INT32 :
                if( check_int_percent( csd[ entry ].data[ csd[ entry ].no_samples ].int_32bit, csd[ entry ].last_value.int_32bit, csb[ entry ].percent_change_to_send ) )
                    percent_change_detected = true;
                break;
            case IMX_FLOAT :
                if( check_float_percent( csd[ entry ].data[ csd[ entry ].no_samples ].float_32bit, csd[ entry ].last_value.float_32bit, csb[ entry ].percent_change_to_send ) )
                    percent_change_detected = true;
                break;
            case IMX_VARIABLE_LENGTH :
                break;
            case IMX_UINT32 :
            default :
                if( check_uint_percent( csd[ entry ].data[ csd[ entry ].no_samples ].uint_32bit, csd[ entry ].last_value.uint_32bit, csb[ entry ].percent_change_to_send ) )
                    percent_change_detected = true;
                break;
        }
    }

    /*
     * Done processing this item - its in the history and saved as current value
     */
    csd[ entry ].no_samples += 1;

    csd[ entry ].last_sample_time = current_time;

    imx_printf( "Event added\r\n" );
    /*
     * See if the batch is ready to go now
     */
    if( ( csd[ entry ].warning != csd[ entry ].last_warning ) ||
        ( ( csd[ entry ].no_samples / 2 ) >= csb[ entry ].sample_batch_size ) ||
        ( csd[ entry ].update_now == true ) ||
        ( percent_change_detected == true ) ) {

        PRINTF( "Setting %s: %u, ID: 0x%08lx to send batch of: %u, batch size %u, sample_now: %s sensor_warning: %u, last: %u, %%change detected: %s\r\n", type == IMX_CONTROLS ? "Control" : "Sensor",
                entry, csb[ entry ].id, csd[ entry ].no_samples, csb[ entry ].sample_batch_size, csd[ entry ].update_now ? "true" : "false",
                csd[ entry ].warning, csd[ entry ].last_warning, percent_change_detected ? "true" : "false" );

        csd[ entry ].update_now = false;
        csd[ entry ].last_warning = csd[ entry ].warning;
        csd[ entry ].send_batch = true;    // Send this now
    }
    /*
     * see if change in error or all we are getting is errors. - Only send once per batch
     */
    if( ( csd[ entry ].error != csd[ entry ].last_error ) ||
        ( imx_is_later( current_time, csd[ entry ].last_sample_time + (wiced_time_t) ( (uint32_t) device_config.scb[ entry ].sample_batch_size * 1000L  ) ) == true ) ) {
        PRINTF( "Error: %u, Last Error: %u, current_time: %lu, time difference: %lu\r\n", csd[ entry ].error, csd[ entry ].last_error, csd[ entry ].last_sample_time, ( csd[ entry ].last_sample_time + (wiced_time_t) ( (uint32_t) device_config.scb[ entry ].sample_batch_size * 1000L  ) - (uint32_t) current_time )  );
        csd[ entry ].last_sample_time = current_time;
        csd[ entry ].last_error = csd[ entry ].error;
        csd[ entry ].send_on_error = true;
    }
#ifdef PRINT_DEBUGS_FOR_EVENTS_DRIVEN
    if( ( device_config.log_messages & DEBUGS_FOR_EVENTS_DRIVEN ) != 0x00 ) {
        imx_printf( "Event Added Data History now contains: %u Event Samples\r\n", csd[ entry ].no_samples/2 );   // 2 samples per event..
        for( i = 0; i < (csd[ entry ].no_samples ); i+= 2 )
            imx_printf( "Sample: %u, time: %lu, data: 0x%08x\r\n", i, csd[ entry ].data[ i ], csd[ entry ].data[ i + 1] );
    }
#endif

}
