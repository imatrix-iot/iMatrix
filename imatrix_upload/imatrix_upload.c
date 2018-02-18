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
#include "../cli/cli_status.h"
#include "../cli/messages.h"
#include "../networking/utility.h"
#include "../time/ck_time.h"
#include "add_internal.h"
#include "imatrix.h"
#include "imatrix_get_ip.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_IMX_UPLOAD
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_IMX_UPLOAD ) != 0x00 ) imx_printf(__VA_ARGS__)
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
#define MIN_IMATRIX_PACKET	256	    // Arbitrary length
#define MAX_VARIABLE_LENGTH 1024    // Limit to a single UDP Packet for now
#define URI_PATH_LENGTH		20	    // SN is 10 + 4 characters
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
extern IOT_Device_Config_t device_config;	// Defined in device/storage.h
extern iMatrix_Control_Block_t icb;
extern control_sensor_data_t *cd;
extern control_sensor_data_t *sd;

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
	PRINTF( "Starting iMatrix Upload\r\n" );

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
    uint16_t packet_length, current_option_number, options_length, remaining_data_length, no_samples, i, j, k, variable_data_length, data_index, var_data_index;
    bool packet_full, entry_loaded;
    uint32_t foo32bit;
    wiced_utc_time_ms_t upload_utc_ms_time;
    wiced_iso8601_time_t iso8601_time;
    imx_peripheral_type_t type;
    bits_t header_bits;
    upload_data_t *upload_data;
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;
    uint16_t no_items;

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
                PRINTF( "iMatrix Send Registration data\r\n" );
                imatrix.state = IMATRIX_GET_PACKET;
                break;
            }
    	    /*
    		 * Check if we have any data to process at this time - first check critical uploads
    		 */
    	    for( type = IMX_CONTROLS; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
//    	            PRINTF( "%u %s: Current Status @: %lu Seconds (past 1970)\r\n", ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors,
//    	                    ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", current_time );
                SET_CSB_VARS( type );
                no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

                for( i = 0; i < no_items; i++ ) {
        	    	if( ( csb[ i ].enabled == true ) &&  ( csb[ i ].send_imatrix == true ) ) {
                        if( ( csd[ i ].warning >= device_config.send_now_on_warning_level ) &&
                            ( csd[ i ].warning != csd[ i ].last_warning ) &&  // Seen a change
                            ( device_config.send_now_on_warning_level != IMX_INFORMATIONAL ) ){     // Make sure this is actually set to something
                            /*
                             * Only need one
                             */
                            imatrix.tusnami_warning = true;
                            PRINTF( "Found %s: %u in Warning State: %u\r\n" , type == IMX_CONTROLS ? "Control" : "Sensor", i, csd[ i ].warning );
                            imatrix.state = IMATRIX_GET_PACKET;
                            break;
                        }
        	    	}
        		}

        		/*
        		 * See if it is time to check for batches completed
        		 */
        		if( imx_is_later( current_time, imatrix.last_upload_time + device_config.imatrix_batch_check_time ) ) {
            		for( i = 0; i < no_items; i++ ) {
                        if( ( csb[ i ].enabled == true ) &&  ( csb[ i ].send_imatrix == true ) ) {
                            if( ( csd[ i ].send_batch == true ) ||
                                ( csd[ i ].send_on_error == true ) ){
                                /*
                                 * Only need one
                                 */
                                if( csd[ i ].send_batch == true ) {
                                    PRINTF( "Found %s: %u Ready to send batch of: %u, send batch: %s\r\n" ,
                                            type == IMX_CONTROLS ? "Control" : "Sensor", i, csd[ i ].no_samples, csd[ i ].send_batch == true ? "true" : "false" );
                                } else {
                                    PRINTF( "Found %s: %u with error\r\n" , type == IMX_CONTROLS ? "Control" : "Sensor", i );
                                }
                                imatrix.state = IMATRIX_GET_PACKET;
                                break;
                            }
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
    	            icb.imatrix_no_packet_avail = false;
    	            if ( ( imatrix.msg->coap.data_block == NULL ) || ( imatrix.msg->coap.data_block->data == NULL ) ||
    	                 ( imatrix.msg->coap.data_block->release_list_for_data_size == NULL ) ) {
    	                imx_printf( "Created invalid message data block in imatrix_history_upload function - Memory Leak.\r\n" );
    	                imatrix.state = IMATRIX_INIT;
    	                return;
    	            } else
    	                imatrix.state = IMATRIX_LOAD_PACKET;
    	        } else {
    	            if( icb.imatrix_no_packet_avail == false ) {
    	                /*
    	                 * Just output message once
    	                 */
    	                imx_printf( "No packet available for iMatrix upload (%u Bytes)\r\n", packet_length );
    	                icb.imatrix_no_packet_avail = true;
    	            }
    	        	/*
    	        	 * Wait a bit before checking again
    	        	 */
    	        	imatrix.last_upload_time = current_time;
    	        }
    		}
    		break;
    	case IMATRIX_LOAD_PACKET :
    	    /*
    	     * There is something to do here. First build the packet
    	     */
    	    imx_set_led( IMX_LED_GREEN, IMX_LED_ON, 0 );         // Set GREEN LED ON Show we are transmitting an iMatrix Packet

    	    PRINTF( "Sending History to iMatrix Server: %03u.%03u.%03u.%03u ",
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0xff000000 ) >> 24 ),
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x00ff0000 ) >> 16 ),
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x0000ff00 ) >> 8 ),
    	            (unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x000000ff ) ) );
    	    if( icb.time_set_with_NTP == true ) {
    	        wiced_time_get_utc_time_ms( &upload_utc_ms_time );
    	    	wiced_time_get_iso8601_time( &iso8601_time );
    	    	PRINTF( "System UTC time is: %.26s\r\n", (char*)&iso8601_time );
    	    	icb.imatrix_upload_count += 1;
    	    } else {
    	    	PRINTF( "System does not have NTP - Sending with 0 for time stamp\r\n" );
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
        	 * Step thru type records - Controls first then Sensors.
        	 *
        	 * Add data to packet that is ready to send
        	 *
        	 */
    	    for( type = IMX_CONTROLS; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {
                SET_CSB_VARS( type );
                no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;
    	    	/*
    	    	 * Step thru record types - Warnings first then regular
    	    	 */
            	for( k = CHECK_WARNING; ( k < NO_TYPE_OF_RECORDS ) && ( packet_full == false ); k++ ) {
            	    PRINTF( "Checking: %s, Type: %s\r\n", type == IMX_CONTROLS ? "Control" : "Sensor", k == CHECK_WARNING ? "Warnings" : "Regular" );
        	        for( i = 0; ( i < no_items ) && ( packet_full == false ); i++ )  {
                        if( ( csb[ i ].enabled == true ) &&  ( csb[ i ].send_imatrix == true ) ) {

                            PRINTF( "Checking %s for %s, No Samples: %u, Warning Level: %u\r\n",
                                    k == CHECK_WARNING ? "Warning" : "Regular", type == IMX_CONTROLS ? device_config.ccb[ i ].name : device_config.scb[ i ].name, csd[ i ].no_samples, csd[ i ].warning );
                            /*
                             * Look first for critical records or errors
                             *
                             * ADD logic for WHILE LOOP to continue processing variable length data if available.
                             */
                            entry_loaded = false;
                            if( ( ( csd[ i ].no_samples > 0 ) && ( csd[ i ].warning != csd[ i ].last_warning ) && ( k == CHECK_WARNING )  ) ||
                                ( ( csd[ i ].no_samples > 0 ) && ( k == CHECK_REGULAR ) ) ||
                                ( csd[ i ].send_on_error == true ) ) {
                                do {
                                    csd[ i ].last_warning = csd[ i ].warning; // Save last warning
                                    csd[ i ].send_on_error = false;        // Not after this send
                                    PRINTF( "%s - %s: %u - Data type: %u ", ( type == IMX_CONTROLS ) ? "Control" : "Sensor", csb[ i ].name, i, csb[ i ].data_type );
                                    if( csb[ i ].data_type == IMX_VARIABLE_LENGTH ) {
                                        /*
                                         * Process Variable Length record
                                         */
                                        if( csb[ i ].sample_rate == 0 ) {
                                            /*
                                             * This is an event entry - timestamp is first item, sample is second entry
                                             */
                                            var_data_index = 1;
                                         } else {
                                             var_data_index = 0;
                                         }

                                        variable_data_length = csd[ i ].data[ var_data_index ].var_data->length; // Events have timestamp / Value pairs
                                        PRINTF( "Trying to add variable length data record, ptr @ 0x%08lx of: %u bytes\r\n", csd[ i ].data[ var_data_index ].uint_32bit, variable_data_length );
                                        if( remaining_data_length >= ( sizeof( header_t ) + variable_data_length ) ) {
                                            /*
                                             * Load data into packet
                                             */
                                            /*
                                             * Load first variable length record
                                             */
                                            PRINTF( "\r\nAdding Variable length data (%u Bytes) for %s: %u - ID: 0x%08lx\r\n", variable_data_length, type == IMX_CONTROLS ? "Control" : "Sensor", i, csb[ i ].id );
                                            /*
                                             * Set up the header and copy in the data
                                             */
                                            upload_data->header.id = htonl( csb[ i ].id );
                                            header_bits.bits.data_type = csb[ i ].data_type;
                                            upload_data->header.sample_rate = htonl( csb[ i ].sample_rate );
                                            if( csb[ i ].sample_rate != 0 ) {
                                                /*
                                                 * These are individual sensor readings over time sample time
                                                 */
                                                header_bits.bits.block_type = type == IMX_CONTROLS ? IMX_BLOCK_CONTROL : IMX_BLOCK_SENSOR;
                                            } else {
                                                /*
                                                 * These are a set of Events
                                                 */
                                                header_bits.bits.block_type = type == IMX_CONTROLS ? IMX_BLOCK_EVENT_CONTROL : IMX_BLOCK_EVENT_SENSOR;
                                                PRINTF( "- Sending Event Block" );
                                            }
                                            header_bits.bits.no_samples = 1;            // Limit to 1 sample - enhance later
                                            header_bits.bits.version = IMATRIX_VERSION_1;
                                            header_bits.bits.warning = csd[ i ].warning;
                                            if( csd[ i ].errors > 0 ) {
                                                header_bits.bits.sensor_error = csd[ i ].error;
                                                csd[ i ].errors = 0;
                                                PRINTF( " --- Errors detected with this entry: %u", (uint16_t) csd[ i ].error );
                                            } else
                                                header_bits.bits.sensor_error = 0;
                                            PRINTF( "\r\n" );
                                            header_bits.bits.reserved = 0;
                                            upload_data->header.bits.bit_data = htonl( header_bits.bit_data );
                                            PRINTF( "Header bits: 0x%08lx\r\n", upload_data->header.bits.bit_data );
                                            upload_data->header.last_utc_ms_sample_time = 0;
                                            if( icb.time_set_with_NTP == true )
                                                upload_data->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );

                                            data_index = 0;
                                            /*
                                             * Add Timestamp if Event data and select base of data
                                             */
                                            if( csb[ i ].sample_rate == 0 ) {
                                                /*
                                                 * This is an event entry - put timestamp in first
                                                 */
                                                upload_data->data[ data_index++ ].uint_32bit = htonl( csd[ i ].data[ 0 ].uint_32bit );
                                                no_samples = 2; // Two samples for event data
                                             } else {
                                                no_samples = 1; // One sample for Time Series data
                                             }
                                            data_ptr =  csd[ i ].data[ var_data_index ].var_data->data;
                                            /*
                                             * Data for variable length data is stored in a structure with the length in the header.
                                             */
                                            upload_data->data[ data_index++ ].uint_32bit = htonl( (uint32_t) variable_data_length );
                                            /*
                                             * Copy Data
                                             */
    /*                                      PRINTF( "Variable length data: " );
                                            uint16_t l;
                                            for( l = 0; l < variable_data_length; l++ )
                                                PRINTF( "[0x%02x]", data_ptr[ l ] );
                                            PRINTF( "\r\n" );
    */
                                            memcpy( &upload_data->data[ data_index ], data_ptr, variable_data_length );
                                            /*
                                             * Now this data is loaded in structure, free up resources
                                             */
                                            PRINTF( "About to free data\r\n" );
                                            imx_add_var_free_pool( csd[ i ].data[ var_data_index ].var_data );
                                            /*
                                             * Move up the data and re calculate the last sample time
                                             */
                                            if( csd[ i ].no_samples == no_samples ) {
                                                csd[ i ].no_samples = 0;   // No need to move any data
                                                csd[ i ].send_batch = false;
                                            } else {
                                                /*
                                                 * Move data up and free up variable data records
                                                 */
                                                memmove( &csd[ i ].data[ 0 ].uint_32bit, &csd[ i ].data[ no_samples ].uint_32bit, SAMPLE_LENGTH * no_samples );
                                                upload_data->header.last_utc_ms_sample_time = htonll( (uint64_t) upload_utc_ms_time - ( csb[ i ].sample_rate * ( csd[ i ].no_samples - no_samples ) ) );
                                                csd[ i ].no_samples = csd[ i ].no_samples - no_samples;
                                            }
                                            /*
                                            * Update the pointer and amount number of bytes left in buffer
                                            *
                                            * Amount = ?time stamp ( 4 bytes ) + data length ( 4 bytes ) + actual variable length data + padding to fill out 32 bits.
                                            */
                                            foo32bit = sizeof( header_t )
                                                    + ( ( csb[ i ].sample_rate == 0 ) ? SAMPLE_LENGTH : 0 ) // Timestamp
                                                    + SAMPLE_LENGTH                                         // Data length
                                                    + variable_data_length;                                  // Data + padding
                                                    // + ( ( variable_data_length % SAMPLE_LENGTH == 0 ) ? 0 : ( SAMPLE_LENGTH - ( variable_data_length % SAMPLE_LENGTH ) ) );
                                            upload_data = ( upload_data_t *) ( ( uint32_t) ( upload_data ) + foo32bit );
                                            remaining_data_length -= foo32bit;
                                            entry_loaded = true;
                                            PRINTF( "Added %lu Bytes, index @: 0x%08lx  , %u Bytes remaining in packet\r\n", foo32bit, (uint32_t) upload_data, remaining_data_length );
                                        } else {
                                            /*
                                             *  Can not fit in this packet
                                             */
                                            packet_full = true;
                                            /*
                                             * Check for long variable length packet that is too big for ANY UDP packet, and discard
                                             */
                                            if( (variable_data_length >= MAX_VARIABLE_LENGTH ) ) {
                                                imx_printf( "Discarding Variable length data, too long to process, %u Bytes\r\n", variable_data_length );
                                                /*
                                                 * If it is not the current value free up resources
                                                 */
                                                if( csd[ i ].last_value.var_data != csd[ i ].data[ var_data_index ].var_data ) {
                                                    PRINTF( "About to free data\r\n" );
                                                    imx_add_var_free_pool( csd[ i ].data[ var_data_index ].var_data );
                                                    /*
                                                     * Move data up in history
                                                     */
                                                    memmove( &csd[ i ].data[ 0 ].uint_32bit, &csd[ i ].data[ 1 ].uint_32bit, SAMPLE_LENGTH * 1 );
                                                }
                                                /*
                                                 * Sanity check
                                                 */
                                                if( csd[ i ].no_samples > 0 )
                                                    csd[ i ].no_samples -= 1;
                                            }
                                        }
                                        if( csd[ i ].no_samples == 0 )
                                            csd[ i ].send_batch = false;
                                    } else {
                                        /*
                                         * Process Regular data record, samplings and events
                                         */
                                        if( remaining_data_length >= ( sizeof( header_t ) + SAMPLE_LENGTH ) ) {
                                            /*
                                             * Process a regular set of samples
                                             */
                                            /*
                                             * See how many we can fit
                                             */
                                            PRINTF( "Checking to see if %u samples can fit in %u", csd[ i ].no_samples, remaining_data_length );
                                            if( remaining_data_length >= ( sizeof( header_t ) + ( SAMPLE_LENGTH * csd[ i ].no_samples ) ) ) {
                                                /*
                                                 * They can all fit
                                                 */
                                                no_samples = csd[ i ].no_samples;
                                                PRINTF( " *** - ALL Can Fit: %u\r\n", no_samples );
                                            } else {
                                                /*
                                                 * Calculate how many will fit
                                                 */
                                                no_samples = ( remaining_data_length - sizeof( header_t ) ) / ( SAMPLE_LENGTH );
                                                PRINTF( " *** - Can Can Fit: %u\r\n", no_samples );
                                            }
                                            PRINTF( "Adding %u samples for %s: %u - ID: 0x%08lx ", no_samples, type == IMX_CONTROLS ? "Control" : "Sensor", i, csb[ i ].id );
                                            /*
                                             * Set up the header and copy in the samples that will fit
                                             */
                                            upload_data->header.id = htonl( csb[ i ].id );
                                            header_bits.bits.data_type = csb[ i ].data_type;
                                            upload_data->header.sample_rate = htonl( csb[ i ].sample_rate );
                                            if( csb[ i ].sample_rate == 0 ) {
                                                /*
                                                 * These are a set of Events
                                                 */
                                                header_bits.bits.block_type = type == IMX_CONTROLS ? IMX_BLOCK_EVENT_CONTROL : IMX_BLOCK_EVENT_SENSOR;
                                                PRINTF( "- Sending Event Block" );
                                            } else {
                                                /*
                                                 * These are individual sensor readings over time sample time
                                                 */
                                                header_bits.bits.block_type = type == IMX_CONTROLS ? IMX_BLOCK_CONTROL : IMX_BLOCK_SENSOR;
                                            }

                                            header_bits.bits.no_samples = no_samples;
                                            header_bits.bits.version = IMATRIX_VERSION_1;
                                            header_bits.bits.warning = csd[ i ].warning;
                                            if( csd[ i ].errors > 0 ) {
                                                header_bits.bits.sensor_error = csd[ i ].error;
                                                csd[ i ].errors = 0;
                                                PRINTF( " --- Errors detected with this entry: %u", (uint16_t) csd[ i ].error );
                                            } else
                                                header_bits.bits.sensor_error = 0;
                                            PRINTF( "\r\n" );
                                            header_bits.bits.reserved = 0;
                                            upload_data->header.bits.bit_data = htonl( header_bits.bit_data );
                                            upload_data->header.last_utc_ms_sample_time = 0;
                                            if( icb.time_set_with_NTP == true )
                                                upload_data->header.last_utc_ms_sample_time = htonll( upload_utc_ms_time );
                                            PRINTF( "Header bits: 0x%08lx, id: 0x%08lx, Sample Rate: %ld\r\n", upload_data->header.bits.bit_data, upload_data->header.id, upload_data->header.sample_rate );

                                            for( j = 0; j < no_samples; j++ ) {
                                                /*
                                                 * Raw copy the data so sign & float are not cast
                                                 */
                                                memcpy( &foo32bit, &csd[ i ].data[ j ].uint_32bit, SAMPLE_LENGTH );
                                                upload_data->data[ j ].uint_32bit = htonl( foo32bit );
                                            }
                                            /*
                                             * Update the structure based on how many items were used
                                             */
                                            if( no_samples == csd[ i ].no_samples ) {
                                                csd[ i ].no_samples = 0;
                                                csd[ i ].send_batch = false;
                                            } else {
                                                /*
                                                 * Move up the data and re calculate the last sample time
                                                 */
                                                memmove( &csd[ i ].data[ 0 ].uint_32bit, &csd[ i ].data[ no_samples ].uint_32bit, SAMPLE_LENGTH * no_samples );
                                                upload_data->header.last_utc_ms_sample_time = htonll( (uint64_t) upload_utc_ms_time - ( csb[ i ].sample_rate * ( csd[ i ].no_samples - no_samples ) ) );
                                                csd[ i ].no_samples = csd[ i ].no_samples - no_samples;
                                                entry_loaded = true;
                                            }
                                            /*
                                            * Update the pointer and amount number of bytes left in buffer
                                            */
                                            foo32bit = sizeof( header_t ) + ( SAMPLE_LENGTH * no_samples );
                                            upload_data = ( upload_data_t *) ( ( uint32_t) ( upload_data ) + foo32bit );
                                            remaining_data_length -= foo32bit;
                                            PRINTF( "Added %lu Bytes, index @: 0x%08lx, %u Bytes remaining in packet\r\n", foo32bit, (uint32_t) upload_data, remaining_data_length );
#ifdef PRINT_DEBUGS_FOR_IMX_UPLOAD
                                            if( ( device_config.log_messages & DEBUGS_FOR_IMX_UPLOAD ) != 0x00 ) {
                                                PRINTF( "Data @: 0x%08lx\r\n", (uint32_t) imatrix.msg->coap.data_block->data );
                                                PRINTF( "Message DATA as string:" );
                                                if (imatrix.msg->coap.data_block != NULL ) {
                                                    for( i = 0; i < max_packet_size() - remaining_data_length; i++ ) {
                                                        if ( isprint( imatrix.msg->coap.data_block->data[ i ] ) ) {
                                                            PRINTF( " %c  ", imatrix.msg->coap.data_block->data[ i ] );
                                                        }
                                                        else {
                                                            PRINTF( " *  " );
                                                        }
                                                    }
                                                    PRINTF( "\r\n" );
                                                    PRINTF( "Message DATA as hex:   " );
                                                    for( i = 0; i < max_packet_size() - remaining_data_length; i++ ) {
                                                        PRINTF( "[%02x]", imatrix.msg->coap.data_block->data[ i ] );
                                                    }
                                                }
                                            }
                                            PRINTF( "\r\n" );
#endif
                                    } else {
                                            packet_full = true;
                                        }
                                        entry_loaded = true;
                                    }

                                    if( packet_full == true )
                                        PRINTF( "\r\niMatrix Packet FULL\r\n" );
                                } while( ( packet_full == false ) && (entry_loaded == false ) );   /* Add logic for multiple variable length data processing */
                            } else {
                                /*
                                 * Nothing matched in this entry
                                 */
                                /*
                                if( k == CHECK_WARNING )
                                    PRINTF( "No Warning Data for %s: %u\r\n", type == IMX_CONTROLS ? device_config.ccb[ i ].name : device_config.scb[ i ].name, i );
                                else
                                    PRINTF( "No History Data for %s: %u\r\n", type == IMX_CONTROLS ? device_config.ccb[ i ].name : device_config.scb[ i ].name, i );
                                */
                            }
                        }
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
    	    PRINTF( "Time Series Data message added to queue\r\n" );
	        imatrix.state = IMATRIX_UPLOAD_COMPLETE;
    		break;
    	case IMATRIX_UPLOAD_COMPLETE :
    	    imx_set_led( IMX_LED_GREEN, IMX_LED_OFF, 0 );         // Set GREEN LED off - Packet sent
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
	wiced_time_t current_time;
    imx_peripheral_type_t type;
    control_sensor_data_t *csd;
    imx_control_sensor_block_t *csb;
    uint16_t no_items;

    wiced_time_get_time( &current_time );

	imx_cli_print( "iMatrix Max Batch Buffer length: %u entries, iMatrix state: ", device_config.history_size );
    switch( imatrix.state ) {
    	case IMATRIX_INIT :
    		imx_cli_print( "Initializing - checking for ready to upload @%lu, ", (uint32_t) current_time );
    		imx_cli_print( "Current Control/Sensor Data pending upload:\r\n" );
    	    for( type = IMX_CONTROLS; type < IMX_NO_PERIPHERAL_TYPES; type++ ) {

                SET_CSB_VARS( type );
                no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

    	        imx_cli_print( "%u %s: Current Status @: %lu mS (mS Timer), Upload time + Check time: %lu\r\n",
    	                ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors,
    	                ( type == IMX_CONTROLS ) ? "Controls" : "Sensors", current_time, (uint32_t) imatrix.last_upload_time + device_config.imatrix_batch_check_time );
    	        no_items = ( type == IMX_CONTROLS ) ? device_config.no_controls : device_config.no_sensors;

    	        for( i = 0; i < no_items; i++ ) {
    	            if( ( csb[ i ].enabled == true ) && ( csb[ i ].send_imatrix == true ) ) {
    	                imx_cli_print( "No: %2u: 0x%08lx: %32s ", i, csb[ i ].id, csb[ i ].name );
    	                if( csd[ i ].no_samples > 0 ) {
                            if( csb[ i ].sample_rate == 0 ) {
                                imx_cli_print( "Event Driven: " );
                                for( j = 0; j < csd[ i ].no_samples; j += 2 ) {
                                    imx_cli_print( "@ %lu, ", csd[ i ].data[ j ].uint_32bit );
                                    switch( csb[ i ].data_type ) {
                                        case IMX_UINT32 :
                                            imx_cli_print( "%lu ", csd[ i ].data[ j + 1 ].uint_32bit );
                                            break;
                                        case IMX_INT32 :
                                            imx_cli_print( "%ld ", csd[ i ].data[ j + 1 ].int_32bit );
                                            break;
                                        case IMX_FLOAT :
                                            imx_cli_print( "%f ", csd[ i ].data[ j + 1 ].float_32bit );
                                            break;
                                        case IMX_VARIABLE_LENGTH :
                                            imx_cli_print( "[%u] ", csd[ i ].last_value.var_data->length );
                                            print_var_data( VR_DATA_STRING, csd[ i ].data[ j + 1 ].var_data );
                                            imx_cli_print( " " );
                                            break;
                                    }
                                }
                            } else {
                                for( j = 0; j < csd[ i ].no_samples; j++ ) {
                                    switch( csb[ i ].data_type ) {
                                        case IMX_UINT32 :
                                            imx_cli_print( "%lu ", csd[ i ].data[ j ].uint_32bit );
                                            break;
                                        case IMX_INT32 :
                                            imx_cli_print( "%ld ", csd[ i ].data[ j ].int_32bit );
                                            break;
                                        case IMX_FLOAT :
                                            imx_cli_print( "%f ", csd[ i ].data[ j ].float_32bit );
                                            break;
                                        case IMX_VARIABLE_LENGTH :
                                            print_var_data( VR_DATA_STRING, csd[ i ].last_value.var_data );
                                            imx_cli_print( " " );
                                            break;
                                    }
                                }

                            }
    	                } else {
    	                    imx_cli_print( "No Samples stored, " );
    	                    if( csb[ i ].sample_rate == 0 )
    	                        imx_cli_print( "Event Driven" );
    	                    else {
    	                        if( csd[ i ].valid == true )
    	                            imx_cli_print( "next sample due @ %lu mSec", ( (uint32_t) csd[ i ].last_sample_time + ( csb[ i ].sample_rate ) ) - (uint32_t) current_time );
    	                        else
    	                            imx_cli_print( "Waiting for first sample" );
    	                    }
    	                }
    	                imx_cli_print( "\r\n" );
    	            }
    	        }
    		}
    		break;
    	case IMATRIX_LOAD_PACKET :
    		imx_cli_print( "Pending upload #: %lu\r\n", icb.imatrix_upload_count );
    		break;
    	case IMATRIX_UPLOAD_COMPLETE :
    		imx_cli_print( "History sending complete\r\n" );
    		break;
    	default:
    		imx_cli_print( "Unknown\r\n" );
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
	imx_cli_print( "iMatrix is @: %s on IP: %u.%u.%u.%u", device_config.imatrix_public_url,
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0xff000000 ) >> 24 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x00ff0000 ) >> 16 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x0000ff00 ) >> 8 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x000000ff ) ) );
	imx_cli_print( " iMatrix Uploads: %lu, upload check interval: %lu\r\n", icb.imatrix_upload_count, device_config.imatrix_batch_check_time );

}
