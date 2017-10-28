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
/** @file imatrix_upload.c
 *
 *
 *  Created on: Dec, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../device/icb_def.h"
#include "../device/config.h"
#include "../device/var_data.h"
#include "../device/hal_leds.h"
#include "../coap/coap.h"
#include "../coap/coap_transmit.h"
#include "../coap/que_manager.h"
#include "../coap/add_coap_option.h"
#include "../cli/interface.h"
#include "../networking/utility.h"
#include "../time/ck_time.h"
#include "add_internal.h"
#include "imatrix.h"
#include "imatrix_get_ip.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_HISTORY
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_HISTORY ) != 0x00 ) st_log_imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

/******************************************************
 *                    Constants
 ******************************************************/
#define MIN_IMATRIX_PACKET	256	// Arbitrary length
#define URI_PATH_LENGTH		20	// SN is 10 + 4 characters
/******************************************************
 *                   Enumerations
 ******************************************************/
enum imatrix_states {
	IMATRIX_INIT = 0,
	IMATRIX_GET_PACKET,
	IMATRIX_LOAD_PACKET,
	IMATRIX_UPLOAD_DATA,
	IMATRIX_UPLOAD_COMPLETE
};

enum {
	CHECK_WARNING = 0,
	CHECK_REGULAR,
	NO_TYPE_OF_RECORDS
};

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct {
	uint8_t	*data_ptr;
	uint16_t state, sensor_no;
	wiced_time_t last_upload_time;
	message_t *msg;
	unsigned int tusnami_warning : 1;
} imatrix_data_t;
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
extern message_list_t list_udp_coap_xmit;
extern uint16_t message_id;
extern uint32_t request_id;
extern IOT_Device_Config_t device_config;	// Defined in device\config.h
extern iMatrix_Control_Block_t icb;
extern control_sensor_data_t cd[ MAX_NO_CONTROLS ];
extern control_sensor_data_t sd[ MAX_NO_SENSORS ];

static imatrix_data_t imatrix;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Set the initialization state
  * @param  None
  * @retval : None
  */
void init_imatrix(void)
{
	imatrix.state = IMATRIX_INIT;
	wiced_time_get_time( &imatrix.last_upload_time );
}

/**
  * @brief  Force iMatrix to start
  * @param  None
  * @retval : None
  */
void start_imatrix(void)
{
	/*
	 * Force a start
	 */
	if( icb.imatrix_public_ip_address.ip.v4 == 0 ) {
		if( get_imatrix_ip_address( 0 ) == false ) {
			imx_printf( "Unable to start iMatrix Upload - Can't get IP address\r\n" );
			return;
		}
	}
	imx_printf( "Starting iMatrix Upload\r\n" );

	imatrix.state = IMATRIX_GET_PACKET;
}

/**
  * @brief  imatrix_upload - upload sensor data for the last period of minutes
  * @param  upload period
  * @retval : None
  */
void imatrix_upload(wiced_time_t current_time)
{
    const uint16_t token_length = 4;
    const uint16_t max_options_length = 30;

    uint8_t options[ max_options_length ], uri_path[ URI_PATH_LENGTH ], *data_ptr;
    uint16_t packet_length, current_option_number, options_length, remaining_data_length, no_samples, i, j, k, item_count, peripheral, variable_data_length, data_index, var_data_ptr;
    bool packet_full, variable_length_data;
    uint32_t foo32bit;
    wiced_utc_time_ms_t upload_utc_ms_time;
    wiced_iso8601_time_t iso8601_time;
    bits_t header_bits;
    upload_data_t *upload_data;
    imx_control_sensor_block_t *csb;    // Temp pointer to control structure
    control_sensor_data_t *data;    // Temp pointer to data

    variable_data_length = 0;

    switch( imatrix.state ) {
    	case IMATRIX_INIT :
    	    /*
    	     * Make sure we can actually do something, device must be provisioned with SN
    	     */
    	    if( ( device_config.AP_setup_mode == true ) || ( icb.wifi_up == false ) || ( strlen( (char *) &device_config.device_serial_number ) == 0 ) || device_config.connected_to_imatrix == false ) {
    	    	// Not online so there is no need to send messages to iMatrix.
    	    	return;
    	    }
    	    imatrix.tusnami_warning = false;
            if( icb.send_registration == true ) {   // Are we registering?
                imx_printf( "iMatrix Send Registration data\r\n" );
                imatrix.state = IMATRIX_GET_PACKET;
                break;
            }
    	    /*
    		 * Check if we have any data to process at this time - first check critical uploads
    		 */
    	    for( peripheral = 0; peripheral < IMX_NO_PERIPHERAL_TYPES; peripheral++ ) {
    	    	if( peripheral == IMX_CONTROLS ) {
    	    		item_count = device_config.no_controls;
    	    	} else {
    	    		item_count = device_config.no_sensors;
    	    	}
        		for( i = 0; i < item_count; i++ ) {
        	    	if( peripheral == IMX_CONTROLS ) {
        	    		data = &cd[ i ];
        	    	} else {
        	    		data = &sd[ i ];
        	    	}
        			if( ( data->warning >= device_config.send_now_on_warning_level ) &&
        				( data->warning != data->last_warning ) &&	// Seen a change
        				( device_config.send_now_on_warning_level != IMX_INFORMATIONAL ) ){		// Make sure this is actually set to something
        				/*
        				 * Only need one
        				 */
        				imatrix.tusnami_warning = true;
        				imx_printf( "Found %s: %u in Warning State: %u\r\n" , peripheral == IMX_CONTROLS ? "Control" : "Sensor", i, data->warning );
        				imatrix.state = IMATRIX_GET_PACKET;
        				break;
        			}
        		}

        		/*
        		 * See if it is time to check for batches completed
        		 */
        		if( is_later( current_time, imatrix.last_upload_time + device_config.imatrix_batch_check_time ) ) {
            		for( i = 0; i < item_count; i++ ) {
            	    	if( peripheral == IMX_CONTROLS ) {
            	    		data = &cd[ i ];
            	    	} else {
            	    		data = &sd[ i ];
            	    	}
            			if( ( data->send_batch == true ) ||
            				( data->send_on_error == true ) ){
            				/*
            				 * Only need one
            				 */
            				if( data->send_batch == true ) {
            					imx_printf( "Found %s: %u Ready to send batch of: %u, send batch: %s\r\n" ,
            					        peripheral == IMX_CONTROLS ? "Control" : "Sensor", i, data->no_samples, data->send_batch == true ? "true" : "false" );
            				} else {
            					imx_printf( "Found %s: %u with error\r\n" , i );
            				}
            				imatrix.state = IMATRIX_GET_PACKET;
            				break;
            			}
            		}
        		}

    	    }
    		break;
    	case IMATRIX_GET_PACKET :	// There is data to process get a packet to put it in
    	    if( /* icb.imatrix_public_ip_address.ip.v4 == 0 */ 1 ) {	// DO it every time for dev
    	    	if( get_imatrix_ip_address( 0 ) == false )
    	    		return;
    	    }
    		packet_length = max_packet_size() ;
    		if( packet_length < MIN_IMATRIX_PACKET )
    			imatrix.state = IMATRIX_INIT;	// Keep restarting as can't work with tiny packets
    		else {
    	        imatrix.msg = msg_get( packet_length );
    	        if( imatrix.msg != NULL ) {		// Got one lets fill it
    	            if ( ( imatrix.msg->coap.data_block == NULL ) || ( imatrix.msg->coap.data_block->data == NULL ) ||
    	                 ( imatrix.msg->coap.data_block->release_list_for_data_size == NULL ) ) {
    	                imx_printf( "Created invalid message data block in imatrix_history_upload function - Memory Leak.\r\n" );
    	                imatrix.state = IMATRIX_INIT;
    	                return;
    	            } else
    	                imatrix.state = IMATRIX_LOAD_PACKET;
    	        } else {
    	        	imx_printf( "No packet available for iMatrix upload\r\n" );
    	        	/*
    	        	 * Wait a bit before checking again
    	        	 */
    	        	imatrix.last_upload_time = current_time;
    	        }
    		}
    		break;
    	case IMATRIX_LOAD_PACKET :
    	    imx_set_led( IMX_LED_GREEN, IMX_LED_ON );         // Set GREEN LED ON Show we are transmitting an iMatrix Packet
    	    imx_printf( "Sending History to iMatrix Server: %03u.%03u.%03u.%03u ",
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0xff000000 ) >> 24 ),
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x00ff0000 ) >> 16 ),
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x0000ff00 ) >> 8 ),
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x000000ff ) ) );
    	    if( icb.time_set_with_NTP == true ) {
    	        wiced_time_get_utc_time_ms( &upload_utc_ms_time );
    	    	wiced_time_get_iso8601_time( &iso8601_time );
    	    	imx_printf( "System UTC time is: %.26s\r\n", (char*)&iso8601_time );
    	    	icb.imatrix_upload_count += 1;
    	    } else {
    	    	imx_printf( "System does not have NTP - Sending with 0 for time stamp\r\n" );
    	    	upload_utc_ms_time = 0;
    	    }
    		imatrix.sensor_no = 0;
    		imatrix.last_upload_time = current_time;

	        /*
	         * Prepare a msg
	         */
	        memset( options, 0, max_options_length );
	        current_option_number = 0;
	        options_length = 0;

	        /*
	         * The following options must be added in numerical order by the first parameter option number
	         *
	         * The URI selected is determined if this is normal upload or a warning upload
	         * isc - iMatrix Sensor Collection
	         * itw - iMatrix Tsunami Warning
	         *
	         */
	        if( imatrix.tusnami_warning == true )
	        	sprintf( (char *) &uri_path,"itw/%lu/%s", device_config.manufactuer_id, (char *) &device_config.device_serial_number );
	        else
	        	sprintf( (char *) &uri_path,"isc/%lu/%s", device_config.manufactuer_id, (char *) &device_config.device_serial_number );
	        options_length = add_coap_str_option( URI_PATH, &current_option_number, (char *) &uri_path, options, max_options_length );
	        options_length += add_coap_uint_option( CONTENT_FORMAT, BINARY_MEDIA_TYPE, &current_option_number,
	        		options + options_length, max_options_length - options_length );

	        //payload_index = 0;
	        imatrix.msg->coap.header.ver = 1;
	        imatrix.msg->coap.header.t = CONFIRMABLE;	// We want to make sure this get here and get payload back with any actions to do
	        imatrix.msg->coap.header.code = ( REQUEST << 5 ) | PUT;
	        imatrix.msg->coap.header.id = message_id++;
	        memset( imatrix.msg->coap.data_block->data, 0x00, imatrix.msg->coap.data_block->release_list_for_data_size->data_size );

	        /*
	         * Add the tokens to the URI
	         */
	        imatrix.msg->coap.header.tkl = token_length;   // We use 4 byte Tokens
	        /*
	         * Add a unique token
	         */
	        request_id += 1;
	        memmove( imatrix.msg->coap.data_block->data, &request_id, token_length );
	        memmove( &imatrix.msg->coap.data_block->data[ token_length ], options, options_length );
	        imatrix.msg->coap.data_block->data[ options_length + token_length ] = PAYLOAD_START;// 0xFF marks the beginning of the payload.

	        remaining_data_length = imatrix.msg->coap.data_block->release_list_for_data_size->data_size - ( token_length + options_length + 1 );
        	upload_data = ( upload_data_t *) &imatrix.msg->coap.data_block->data[ token_length + options_length + 1 ];
        	packet_full = false;
        	/*
        	 * Check if we are sending a registration request
        	 */
            if( ( icb.send_registration == true ) && ( imatrix.tusnami_warning == false ) ) {
                icb.send_registration = false;
                add_registration( &upload_data, &remaining_data_length, upload_utc_ms_time );
            }
        	/*
        	 * Check if we need to send current location information
        	 */
        	if( ( icb.send_gps_coords == true ) && ( imatrix.tusnami_warning == false ) ) {
        		/*
        		 * We assume that there is enough space in a new packet to fit this data
        		 */
        		icb.send_gps_coords = false;
        		add_gps( &upload_data, &remaining_data_length, upload_utc_ms_time );
        	}
        	if( ( icb.send_indoor_coords == true ) && ( imatrix.tusnami_warning == false ) ) {
        		icb.send_indoor_coords = false;
        		add_indoor_location( &upload_data, &remaining_data_length, upload_utc_ms_time );
        	}
        	/*
        	 * Step thru peripheral records - Controls first then Sensors
        	 */
    	    for( peripheral = 0; peripheral < IMX_NO_PERIPHERAL_TYPES; peripheral++ ) {
    	    	/*
    	    	 * Step thru record types - Warnings first then regular
    	    	 */
    	    	if( peripheral == IMX_CONTROLS ) {
    	    		item_count = device_config.no_controls;
    	    	} else {
    	    		item_count = device_config.no_sensors;
    	    	}
            	for( k = 0; ( k < NO_TYPE_OF_RECORDS ) && ( packet_full == false ); k++ ) {
            	    // imx_printf( "Checking: %s, Type: %s\r\n", peripheral == CONTROLS ? "Control" : "Sensor", k == CHECK_WARNING ? "Warnings" : "Regular" );
        	        for( i = 0; ( i < item_count ) && ( packet_full == false ); i++ )  {
            	    	if( peripheral == IMX_CONTROLS ) {
            	    		data = &cd[ i ];
                            csb = &device_config.ccb[ i ];
                            // imx_printf( "Control: %u - Data type: %u\r\n", i, device_config.ccb[ i ].data_type );
            	    		if( device_config.ccb[ i ].data_type == IMX_DO_VARIABLE_LENGTH ) {
            	    		    variable_length_data = true;
            	    		} else
            	    		    variable_length_data = false;
            	    	} else {
            	    		data = &sd[ i ];
                            csb = &device_config.scb[ i ];
            	    		// imx_printf( "Sensor: %u - Data type: %u\r\n", i, device_config.scb[ i ].data_type );
                            if( device_config.scb[ i ].data_type == IMX_DI_VARIABLE_LENGTH ) {
                                // imx_printf( "Processing Variable length record\r\n" );
                                variable_length_data = true;
                            } else
                                variable_length_data = false;
//            				imx_printf( "iMatrix - Setting Sensor: %u Data to: 0x%08lx\r\n", i, (uint32_t) data );

            	    	}
            	    	/*
            	    	imx_printf( "Checking %s for %s, No Samples: %u, Warning Level: %u\r\n",
            	    			k == CHECK_WARNING ? "Warning" : "Regular", peripheral == CONTROLS ? device_config.ccb[ i ].name : device_config.scb[ i ].name, data->no_samples, data->warning );
            	    	*/
        	        	/*
        	        	 * Look first for critical records or errors
        	        	 *
        	        	 * ADD logic for WHILE LOOP to continue processing variable length data if available.
        	        	 */
            	    	do {
                            if( ( ( data->no_samples > 0 ) && ( data->warning != data->last_warning ) && ( k == CHECK_WARNING )  ) ||
                                ( ( data->no_samples > 0 ) && ( k == CHECK_REGULAR ) ) ||
                                ( data->send_on_error == true ) ) {
                                data->last_warning = data->warning; // Always set
                                data->send_on_error = false;        // Not after this send
                                if( variable_length_data == true )  {
                                    if( csb->sample_rate == 0 )
                                        variable_data_length = data->data[ 1 ].var_data->header.length; // Events have timestamp / Value pairs
                                    else
                                        variable_data_length = data->data[ 0 ].var_data->header.length; // Take first entry
                                }
                                /*
                                 * See if we have space for at least one header and one record or if Variable Data the length of the data for one sample
                                 */
                                imx_printf( "*** Check - Remaining Length %u, Header + 1 sample: %u, Header + all samples: %u",
                                        remaining_data_length, ( sizeof( header_t ) + SAMPLE_LENGTH ),( sizeof( header_t ) + ( SAMPLE_LENGTH * data->no_samples ) ) );
                                if( ( ( variable_length_data == false ) && ( remaining_data_length >= ( sizeof( header_t ) + SAMPLE_LENGTH ) ) ) ||
                                    ( ( variable_length_data == true ) && ( remaining_data_length >= ( sizeof( header_t ) + variable_data_length ) ) ) ) {
                                    /*
                                     * Load data into packet - Can be Variable length or variable number of samples
                                     */
                                    if( variable_length_data == true ) {
                                        /*
                                         * Load first variable length record
                                         */
                                        imx_printf( "\r\nAdding Variable length data (%u Bytes) for %s: %u - ID: 0x%08lx ",
                                                variable_data_length, peripheral == IMX_CONTROLS ? "Control" : "Sensor", i, csb->id );
                                        /*
                                         * Set up the header and copy in the data
                                         */
                                        upload_data->header.id = htonl( csb->id );
                                        header_bits.bits.data_type = csb->data_type;
                                        upload_data->header.sample_rate = htonl( csb->sample_rate );
                                        if( csb->sample_rate != 0 ) {
                                            /*
                                             * These are individual sensor readings over time sample time
                                             */
                                            header_bits.bits.block_type = peripheral == IMX_CONTROLS ? IMX_BLOCK_CONTROL : IMX_BLOCK_SENSOR;
                                        } else {
                                            /*
                                             * These are a set of Events
                                             */
                                            header_bits.bits.block_type = peripheral == IMX_CONTROLS ? IMX_BLOCK_EVENT_CONTROL : IMX_BLOCK_EVENT_SENSOR;
                                            imx_printf( "- Sending Event Block" );
                                        }
                                        header_bits.bits.no_samples = 1;            // Limit to 1 sample - enhance later
                                        header_bits.bits.version = IMATRIX_VERSION_1;
                                        header_bits.bits.warning = data->warning;
                                        if( data->errors > 0 ) {
                                            header_bits.bits.sensor_error = data->error;
                                            data->errors = 0;
                                            imx_printf( " --- Errors detected with this entry: %u", (uint16_t) data->error );
                                        } else
                                            header_bits.bits.sensor_error = 0;
                                        imx_printf( "\r\n" );
                                        header_bits.bits.reserved = 0;
                                        upload_data->header.bits.bit_data = htonl( header_bits.bit_data );
                                        imx_printf( "Header bits: 0x%08lx\r\n", upload_data->header.bits.bit_data );
                                        upload_data->header.last_utc_ms_sample_time = 0;
                                        if( icb.time_set_with_NTP == true )
                                            upload_data->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );

                                        data_index = 0;
                                        /*
                                         * Add Timestamp if Event data and select base of data
                                         */
                                        if( csb->sample_rate == 0 ) {
                                            /*
                                             * This is an event entry - put timestamp in first
                                             */
                                            upload_data->data[ data_index++ ].uint_32bit = htonl( data->data[ 0 ].uint_32bit );
                                            var_data_ptr = 1;
                                            no_samples = 2; // Two samples for event data
                                         } else {
                                             var_data_ptr = 0;
                                            no_samples = 1; // One sample for Time Series data
                                         }
                                        data_ptr =  data->data[ var_data_ptr ].var_data->data;
                                        /*
                                         * Data for variable length data is stored in a structure with the length in the header.
                                         */
                                        upload_data->data[ data_index++ ].uint_32bit = htonl( (uint32_t) variable_data_length );
                                        /*
                                         * Copy Data
                                         */
                                        imx_printf( "Variable length data: " );
                                        uint16_t l;
                                        for( l = 0; l < variable_data_length; l++ )
                                            imx_printf( "[0x%02x]", data_ptr[ l ] );
                                        imx_printf( "\r\n" );

                                        memcpy( &upload_data->data[ data_index ], data_ptr, variable_data_length );
                                        /*
                                         * Now this data is loaded in free up resources and decrement no samples
                                         */
                                        imx_printf( "About to free data\r\n" );
                                        add_var_free_pool( data->data[ var_data_ptr ].var_data );
                                        /*
                                         * Move up the data and re calculate the last sample time
                                         */
                                        if( data->no_samples == no_samples ) {
                                            data->no_samples = 0;   // No need to move any data
                                            data->send_batch = false;
                                        } else {
                                            memmove( &data->data[ 0 ].uint_32bit, &data->data[ no_samples ].uint_32bit, SAMPLE_LENGTH * no_samples );
                                            upload_data->header.last_utc_ms_sample_time = htonll( (uint64_t) upload_utc_ms_time - ( csb->sample_rate * ( data->no_samples - no_samples ) ) );
                                            data->no_samples = data->no_samples - no_samples;
                                        }
                                        /*
                                        * Update the pointer and amount number of bytes left in buffer
                                        *
                                        * Amount = ?time stamp ( 4 bytes ) + data length ( 4 bytes ) + actual variable length data + padding to fill out 32 bits.
                                        */
                                        foo32bit = sizeof( header_t )
                                                + ( ( csb->sample_rate == 0 ) ? SAMPLE_LENGTH : 0 )    // Timestamp
                                                + SAMPLE_LENGTH                                         // Data length
                                                + variable_data_length                                  // Data + padding
                                                + ( ( variable_data_length % SAMPLE_LENGTH == 0 ) ? 0 : ( SAMPLE_LENGTH - ( variable_data_length % SAMPLE_LENGTH ) ) );
                                        imx_printf( "Added %lu Bytes\r\n", foo32bit );
                                        upload_data = ( upload_data_t *) ( uint32_t) ( upload_data ) + foo32bit;
                                        remaining_data_length -= foo32bit;
                                        imx_printf( "Added %lu Bytes, %u Bytes remaining in packet\r\n", foo32bit, remaining_data_length );
                                    } else {
                                        /*
                                         * Process a regular set of samples
                                         */
                                        /*
                                         * See how many we can fit
                                         */
                                        if( remaining_data_length >= ( sizeof( header_t ) + ( SAMPLE_LENGTH * data->no_samples ) ) ) {
                                            /*
                                             * They can all fit
                                             */
                                            no_samples = data->no_samples;
                                            imx_printf( " *** - ALL Can Fit: %u\r\n", no_samples );
                                        } else {
                                            /*
                                             * Calculate how many will fit
                                             */
                                            no_samples = ( remaining_data_length - sizeof( header_t ) ) / ( SAMPLE_LENGTH );
                                            imx_printf( " *** - Can Can Fit: %u\r\n", no_samples );
                                        }
                                        imx_printf( "Adding %u samples for %s: %u - ID: 0x%08lx ",
                                                no_samples, peripheral == IMX_CONTROLS ? "Control" : "Sensor", i, csb->id );
                                        /*
                                         * Set up the header and copy in the samples that will fit
                                         */
                                        upload_data->header.id = htonl( csb->id );
                                        header_bits.bits.data_type = csb->data_type;
                                        upload_data->header.sample_rate = htonl( csb->sample_rate );
                                        if( csb->sample_rate != 0 ) {
                                            /*
                                             * These are individual sensor readings over time sample time
                                             */
                                            header_bits.bits.block_type = peripheral == IMX_CONTROLS ? IMX_BLOCK_CONTROL : IMX_BLOCK_SENSOR;
                                        } else {
                                            /*
                                             * These are a set of Events
                                             */
                                            header_bits.bits.block_type = peripheral == IMX_CONTROLS ? IMX_BLOCK_EVENT_CONTROL : IMX_BLOCK_EVENT_SENSOR;
                                            imx_printf( "- Sending Event Block" );
                                        }

                                        header_bits.bits.no_samples = no_samples;
                                        header_bits.bits.version = IMATRIX_VERSION_1;
                                        header_bits.bits.warning = data->warning;
                                        if( data->errors > 0 ) {
                                            header_bits.bits.sensor_error = data->error;
                                            data->errors = 0;
                                            imx_printf( " --- Errors detected with this entry: %u", (uint16_t) data->error );
                                        } else
                                            header_bits.bits.sensor_error = 0;
                                        imx_printf( "\r\n" );
                                        header_bits.bits.reserved = 0;
                                        upload_data->header.bits.bit_data = htonl( header_bits.bit_data );

                                        upload_data->header.last_utc_ms_sample_time = 0;
                                        if( icb.time_set_with_NTP == true )
                                            upload_data->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );

                                        for( j = 0; j < no_samples; j++ ) {
                                            /*
                                             * Raw copy the data so sign & float are not cast
                                             */
                                            memcpy( &foo32bit, &data->data[ j ].uint_32bit, SAMPLE_LENGTH );
                                            upload_data->data[ j ].uint_32bit = htonl( foo32bit );
                                        }
                                        /*
                                         * Update the structure based on how many items were used
                                         */
                                        if( no_samples == data->no_samples ) {
                                            data->no_samples = 0;
                                            data->send_batch = false;
                                        } else {
                                            /*
                                             * Move up the data and re calculate the last sample time
                                             */
                                            memmove( &data->data[ 0 ].uint_32bit, &data->data[ no_samples ].uint_32bit, SAMPLE_LENGTH * no_samples );
                                            upload_data->header.last_utc_ms_sample_time = htonll( (uint64_t) upload_utc_ms_time - ( csb->sample_rate * ( data->no_samples - no_samples ) ) );
                                            data->no_samples = data->no_samples - no_samples;
                                            packet_full = true;
                                        }
                                        /*
                                        * Update the pointer and amount number of bytes left in buffer
                                        */
                                        foo32bit = sizeof( header_t ) + ( SAMPLE_LENGTH * no_samples );
                                        upload_data = ( upload_data_t *) ( ( uint32_t) ( upload_data ) + foo32bit );
                                        remaining_data_length -= foo32bit;
                                        imx_printf( "Added %lu Bytes, %u Bytes remaining in packet\r\n", foo32bit, remaining_data_length );
                                    }
                                } else {
                                    imx_printf( "\r\niMatrix Packet FULL\r\n" );
                                    packet_full = true;
                                }
                            } else {
                                /*
                                if( k == CHECK_WARNING )
                                    imx_printf( "No Warning Data for %s: %u\r\n", peripheral == CONTROLS ? device_config.ccb[ i ].name : device_config.scb[ i ].name, i );
                                else
                                    imx_printf( "No History Data for %s: %u\r\n", peripheral == CONTROLS ? device_config.ccb[ i ].name : device_config.scb[ i ].name, i );
                                */
                            }
            	    	} while( false && ( packet_full == false ) );   /* Add logic for multiple variable length data processing */
        	        }
            	}
    	    }
        	/*
        	 * Calculate how long the packet really is
        	 * imatrix.msg->coap.header.tkl + options_length + payload + 1;  // Tokens + options_length + 0xff + data_length = msg_length
        	 */
	        imatrix.msg->coap.msg_length = max_packet_size() - remaining_data_length;
	        /*
	         * Fill in the UDP details of where this is going
	         */
	        imatrix.msg->coap.ip_addr.version = WICED_IPV4;
	        imatrix.msg->coap.ip_addr.ip.v4 = icb.imatrix_public_ip_address.ip.v4;

	        imatrix.msg->coap.port = DEFAULT_COAP_PORT;
	        /*
	         * Add this message to the xmit que and start a transmit
	         */
	        print_msg( imatrix.msg );
	        list_add( &list_udp_coap_xmit, imatrix.msg );
    	    imx_printf( "Time Series Data message added to queue\r\n" );
	        imatrix.state = IMATRIX_UPLOAD_COMPLETE;
    		break;
    	case IMATRIX_UPLOAD_COMPLETE :
    	    imx_set_led( IMX_LED_GREEN, IMX_LED_OFF );         // Set GREEN LED off - Packet sent
    	    imatrix.state = IMATRIX_INIT;
    	    break;
    	default:
    	    imatrix.state = IMATRIX_INIT;
    		break;
    }
}

void imatrix_production_upload( uint16_t mode )
{
    const uint16_t token_length = 0;
    const uint16_t max_options_length = 30;
    const char *json_format = "{\"serial_number\":\"%s\",\"mode\":%u}";

    message_t *msg;
    uint16_t current_option_number = 0;
    uint8_t options[ max_options_length ];
    uint16_t options_length = 0;
    uint16_t json_length = 0;
    uint16_t data_size = 0;

    memset( options, 0, max_options_length );

    // The following options must be added in numerical order by the first parameter, option number.
    options_length = add_coap_str_option( URI_PATH, &current_option_number, "production", options, max_options_length );
    options_length += add_coap_uint_option( CONTENT_FORMAT, JSON_MEDIA_TYPE, &current_option_number,
    		options + options_length, max_options_length - options_length );

    // To accurately calculate the length of the json, parameters used in json_format must be identical to those used later.
    json_length = snprintf( NULL, 0, json_format, device_config.sn, mode );

    data_size = token_length + options_length + 1 + json_length;// Tokens + options_length + 0xff + json_length = msg_length

    // Retrieve a message struct.

    // Add 1 to required minimum size to give room for null terminator that might be added by sprintf(.. json_format ..).
    msg = msg_get( data_size + 1 );
    if ( msg == NULL ) {// This is bad.
        imx_printf( "No packet available to send history\r\n" );
        return;
    }
    if ( ( imatrix.msg->coap.data_block == NULL ) || ( imatrix.msg->coap.data_block->data == NULL ) ||
         ( imatrix.msg->coap.data_block->release_list_for_data_size == NULL ) )
    {
    	imx_printf( "Created invalid message data block in imatrix_history_upload function - Memory Leak.\r\n" );
    	return;
    }

    // Store values in the message struct.

    imatrix.msg->coap.header.ver = 1;
    imatrix.msg->coap.header.t = NON_CONFIRMABLE;
    imatrix.msg->coap.header.code = ( REQUEST << 5 ) | PUT;
    imatrix.msg->coap.header.id = message_id++;
    imatrix.msg->coap.header.tkl = token_length;   // Update to support tokens No Tokens - must be zero for the moment.
    imatrix.msg->coap.msg_length = data_size;

    // Fill in the UDP details of where this is going.
    imatrix.msg->coap.ip_addr.version = WICED_IPV4;
    imatrix.msg->coap.ip_addr.ip.v4 = icb.imatrix_public_ip_address.ip.v4;
    imatrix.msg->coap.port = DEFAULT_COAP_PORT;

    // Store values in the data array part of the message struct: token + options + payload marker + JSON payload.
    memset( imatrix.msg->coap.data_block->data, 0x00, imatrix.msg->coap.data_block->release_list_for_data_size->data_size  );
    memmove( imatrix.msg->coap.data_block->data, options, options_length );
    imatrix.msg->coap.data_block->data[ options_length ] = PAYLOAD_START;// 0xFF marks the start of the payload.
    sprintf( (char *) ( imatrix.msg->coap.data_block->data + options_length + 1 ), json_format, device_config.sn, mode );

    // Add this message to the xmit que and start a transmit.

    list_add( &list_udp_coap_xmit, msg );
    coap_transmit( true );	// Send message now
    wiced_rtos_delay_milliseconds( 2000 );	// Let it send

}
/**
  * @brief send a log message to iMatrix Servers
  * @param  string to send
  * @retval : None
  */
void imatrix_log( char *buffer )
{
    const uint16_t token_length = 0;
    const uint16_t max_options_length = 64;
    const char *json_format = "{\"time\":%lu,\"message\":\"%s\"}";
    char uri_str[ max_options_length ];

    message_t *msg;
    uint16_t current_option_number = 0;
    uint8_t options[ max_options_length ];
    uint16_t options_length = 0;
    uint16_t json_length = 0;
    uint16_t data_size = 0;
	wiced_utc_time_t utc_time = 0;

	return;
	if( icb.wifi_up == false )
		return;

	wiced_time_get_utc_time( &utc_time );

    memset( options, 0, max_options_length );

    sprintf( uri_str, "log/%s", (char *) &device_config.sn );
    // The following options must be added in numerical order by the first parameter, option number.
    options_length = add_options_from_string( URI_PATH, '/', uri_str, &current_option_number, options, max_options_length );
    options_length += add_coap_uint_option( CONTENT_FORMAT, JSON_MEDIA_TYPE, &current_option_number,
    		options + options_length, max_options_length - options_length );

    // To accurately calculate the length of the json, parameters used in json_format must be identical to those used later.
    json_length = snprintf( NULL, 0, json_format, (uint32_t) ( utc_time % IMX_SEC_IN_DAY  ), buffer);

    data_size = token_length + options_length + 1 + json_length;// Tokens + options_length + 0xff + json_length = msg_length

    // Retrieve a message struct.

    // Add 1 to required minimum size to give room for null terminator that might be added by sprintf(.. json_format ..).
    msg = msg_get( data_size + 1 );
    if ( msg == NULL ) {// This is bad.
        imx_printf( "No packet available to send history\r\n" );
        return;
    }
    if ( ( imatrix.msg->coap.data_block == NULL ) || ( imatrix.msg->coap.data_block->data == NULL ) ||
         ( imatrix.msg->coap.data_block->release_list_for_data_size == NULL ) )
    {
    	imx_printf( "Created invalid message data block in imatrix_history_upload function - Memory Leak.\r\n" );
    	return;
    }

    // Store values in the message struct.

    imatrix.msg->coap.header.ver = 1;
    imatrix.msg->coap.header.t = NON_CONFIRMABLE;
    imatrix.msg->coap.header.code = ( REQUEST << 5 ) | PUT;
    imatrix.msg->coap.header.id = message_id++;
    imatrix.msg->coap.header.tkl = token_length;   // No Tokens - must be zero for the moment.
    imatrix.msg->coap.msg_length = data_size;

    // Fill in the UDP details of where this is going.
    imatrix.msg->coap.ip_addr.version = WICED_IPV4;
    imatrix.msg->coap.ip_addr.ip.v4 = icb.imatrix_public_ip_address.ip.v4;
    imatrix.msg->coap.port = DEFAULT_COAP_PORT;

    // Store values in the data array part of the message struct: token + options + payload marker + JSON payload.
    memset( imatrix.msg->coap.data_block->data, 0x00, imatrix.msg->coap.data_block->release_list_for_data_size->data_size  );
    memmove( imatrix.msg->coap.data_block->data, options, options_length );
    imatrix.msg->coap.data_block->data[ options_length ] = PAYLOAD_START;// 0xFF marks the start of the payload.
    sprintf( (char *) ( imatrix.msg->coap.data_block->data + options_length + 1 ), json_format, (uint32_t) ( utc_time % IMX_SEC_IN_DAY  ), buffer);

    // Add this message to the xmit que and start a transmit.

    list_add( &list_udp_coap_xmit, msg );
    coap_transmit( true );	// Send message now

}
/**
  * @brief Display current status and time before next upload
  * @param  current time
  * @retval : None
  */
void imatrix_status( uint16_t arg)
{

    UNUSED_PARAMETER( arg );
	uint16_t i, j;

	cli_print( "iMatrix state: " );
    switch( imatrix.state ) {
    	case IMATRIX_INIT :
    		cli_print( "Initializing - checking for ready to upload" );
    		cli_print( "Current Control/Sensor Data pending upload:\r\n" );
    		for( i = 0; i < device_config.no_controls; i++ ) {
    			cli_print( "Control 0x%08lx: %s ", device_config.ccb[ i ].id, device_config.ccb[ i ].name );
    			if( cd[ i ].no_samples > 0 ) {
        			for( j = 0; j < cd[ i ].no_samples; j++ ) {
        				switch( device_config.ccb[ i ].data_type ) {
        					case IMX_DI_UINT32 :
        						cli_print( "%lu ", cd[ i ].data[ j ].uint_32bit );
        						break;
        					case IMX_DI_INT32 :
        						cli_print( "%ld ", cd[ i ].data[ j ].int_32bit );
        						break;
        					case IMX_AI_FLOAT :
        						cli_print( "%f ", cd[ i ].data[ j ].float_32bit );
        						break;
        				}
        			}
    			} else
    				cli_print( "No Samples stored" );
    			cli_print( "\r\n" );
			}
    		for( i = 0; i < device_config.no_sensors; i++ ) {
    			cli_print( "Sensor 0x%08lx: %s ", device_config.scb[ i ].id, device_config.scb[ i ].name );
    			for( j = 0; j < sd[ i ].no_samples; j++ ) {
    				switch( device_config.scb[ i ].data_type ) {
    					case IMX_DI_UINT32 :
    						cli_print( "%lu ", sd[ i ].data[ j ].uint_32bit );
    						break;
    					case IMX_DI_INT32 :
    						cli_print( "%ld ", sd[ i ].data[ j ].int_32bit );
    						break;
    					case IMX_AI_FLOAT :
    						cli_print( "%f ", sd[ i ].data[ j ].float_32bit );
    						break;
    				}
    			}
				cli_print( "\r\n" );
    		}
    		break;
    	case IMATRIX_LOAD_PACKET :
    		cli_print( "Pending upload #: %lu\r\n", icb.imatrix_upload_count );
    		break;
    	case IMATRIX_UPLOAD_COMPLETE :
    		cli_print( "History sending complete\r\n" );
    		break;
    	default:
    		cli_print( "Unknown\r\n" );
    		break;
    }
}
/**
  * @brief Display current status and time before next upload
  * @param  current time
  * @retval : None
  */
void print_imatrix_config(void)
{
	cli_print( "iMatrix is @: %s on IP: %u.%u.%u.%u", device_config.imatrix_public_url,
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0xff000000 ) >> 24 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x00ff0000 ) >> 16 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x0000ff00 ) >> 8 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x000000ff ) ) );
	cli_print( " iMatrix Uploads: %lu, upload check interval: %lu\r\n", icb.imatrix_upload_count, device_config.imatrix_batch_check_time );

}