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
/*
 *  Created on: Apr 14, 2015
 *      Author: eric thelin
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "wiced.h"
#include "../device/icb_def.h"
#include "../coap/coap.h"
#include "../coap/add_coap_option.h"
#include "../coap/que_manager.h"
#include "coap_msg_get_store.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../storage.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_BASIC_MESSAGING
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_BASIC_MESSAGING ) != 0x00 ) imx_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif
/******************************************************
 *               Variable Declarations
 ******************************************************/
extern uint16_t message_id; // See coap_setup.c for variable definition
extern IOT_Device_Config_t device_config;   // Defined in storage.h

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Using a null terminated string as input instead of a uint8_t array with length specified
 * call the binary version of this function that uses the uint8_t array.
 *
 * @param msg_out
 * @param code_in
 * @param type_in
 * @param data_str is a null terminated string
 *
 * written by Eric Thelin mid 2015
 */
wiced_result_t coap_store_response_data(coap_message_t* msg_out, uint16_t code_in,
    uint16_t type_in, char* data_str, uint16_t media_type )
{
    if ( data_str == NULL ) {
        PRINTF( "NULL in parameter of coap_store_response_data function.\r\n" );
        return WICED_ERROR;
    }
    return coap_store_response_binary(msg_out, code_in, type_in, (uint8_t*) data_str, strlen(data_str), media_type );
}

/**
 * This function stores data in a response message without altering parts of the message
 * struct that should be copied from the request message. These parts are left alone:
 *  Message:token length (tkl)
 *         :version (ver)
 *         :remote device ip address
 *         :remote port
 *
 * @param msg_out
 * @param code_in
 * @param type_in
 * @param data to copy into payload of "msg_out"
 * @param size of data
 *
 * written by Eric Thelin 14 April 2015
 */
wiced_result_t coap_store_response_binary( coap_message_t* msg_out, uint16_t code_in,
    uint16_t type_in, uint8_t* data, uint16_t data_size, uint16_t media_type )
{
    if ( msg_out == NULL ) {
        PRINTF( "Null parameter in coap_store_response_binary function.\r\n" );
        return WICED_ERROR;
    }
    if ( ( data == NULL ) && ( data_size > 0 ) ) {
    	PRINTF( "Null data passed to coap_store_response_binary function.\r\n" );
    	return WICED_ERROR;
    }

	wiced_result_t result = WICED_ERROR;
    uint16_t payload_index = 0;
    uint16_t size_used = 0;
    uint16_t bytes_requrired_for_media_type = 0;
    uint16_t preceeding_option_number = 0;
    uint8_t options[ 5 ];

    // Calculate size needed for data.
    size_used = msg_out->header.tkl;
    if ( data_size > 0 ) {
    	bytes_requrired_for_media_type = add_coap_uint_option( CONTENT_FORMAT, media_type,
    			&preceeding_option_number, options, 5 );
    	size_used += bytes_requrired_for_media_type;
    	size_used++;// for payload marker
    	size_used += data_size;
    }
    payload_index = size_used;// Ensure the payload is big enough.
	result = coap_store_response_header( msg_out, code_in, type_in, &payload_index );
	// payload_index is returned with just the data array size used by the header.
	if ( result == WICED_SUCCESS ) {

		// If there is any payload to send add Media Type, payload marker 0xFF and payload.
		if ( data_size > 0 ) {
			memmove( msg_out->data_block->data + payload_index, options, bytes_requrired_for_media_type );

			payload_index += bytes_requrired_for_media_type;
			msg_out->data_block->data[ payload_index ] = PAYLOAD_START;  // Byte that ends list of tokens and options

			payload_index++;
			memmove( msg_out->data_block->data + payload_index, data, data_size );
			msg_out->msg_length = size_used;
			msg_out->has_payload_bit_flag = 1;
		}
	}
    return result;
}

/**
 * Assemble coap header for a response
 * using the information from the request stored in "msg_out" and
 * the parameters "code_in" and "type_in".
 *
 * @param msg_out is the important information from the request, initially.
 *                On exit, "msg_out" is the information for the coap header of the response.
 * @param code_in is the response code to assign.
 * @param type_in is the message type to assign.
 * @param data_size is returned as size of the variable length portion of the header.
 *                  If "data_size" is NULL, it is ignored.
 *                  When " *data_size" has an input value,
 *                  the data block assigned to msg_out will have at least that many bytes.
 * @return WICED_SUCCESS or WICED_ERROR if input failed validation.
 *
 * NOTE: This routine sets msg_out->has_payload_bit_flag to 0, so any
 * append data function called on "msg_out" after this will automatically
 * add a payload marker.
 *
 * NOTE 2: Currently, the token from the request is preserved in the response,
 * but no options are preserved.
 *
 * NOTE 3: Even when there is an input validation failure,
 * all header values are updated for the response as much as possible
 * to facilitate cases when the calling code ignores
 * the success/fail return value.
 *
 * written by Eric Thelin 17 January 2016
 */
wiced_result_t coap_store_response_header( coap_message_t* msg_out, uint16_t code_in, uint16_t type_in, uint16_t *data_size )
{
    // Do not change these msg_out attributes from the request:
    //    udp_ip_addr;
    //    udp_port;
    //    my_ip_from_request;
    //    header.ver
    //    header.tkl

    if ( msg_out == NULL ) {
        PRINTF ( "Invalid CoAP message: NULL.\r\n" );
        return WICED_ERROR;
    }
    msg_out->header.t = type_in;
    msg_out->header.code = code_in;

    msg_out->has_payload_bit_flag = 0;

    msg_out->initial_timestamp = 0;
    msg_out->next_timestamp = 0;
    msg_out->send_attempts = 0;

    msg_out->msg_length = 0;
    if ( 8 < msg_out->header.tkl ) {
        PRINTF("Token length too big in CoAP response.\r\n");
        return WICED_ERROR;
    }
    msg_out->msg_length = msg_out->header.tkl;

    uint16_t min_required_data_block_size = msg_out->msg_length;

    if ( ( data_size != NULL ) && ( *data_size > min_required_data_block_size ) ) {
    	min_required_data_block_size = *data_size;
    }
    if ( coap_msg_resize( msg_out, min_required_data_block_size ) != WICED_SUCCESS ) {
    	PRINTF( "Failed to find large enough data block in coap_store_response_header function.\r\n" );
    	return WICED_ERROR;
    }

    if ( ( type_in == CONFIRMABLE ) || ( type_in == NON_CONFIRMABLE ) ) {
        msg_out->header.id = message_id++;
    }
    // For ACK & RESET keep header_id from request. Any other type_in is invalid.
    else if ( ( type_in != ACKNOWLEDGEMENT ) && ( type_in != RESET ) ) {
        PRINTF("Invalid response type in CoAP message.\r\n");
        return WICED_ERROR;
    }

    if ( data_size != NULL ) {
        *data_size = msg_out->header.tkl;
    }
    return WICED_SUCCESS;
}

uint16_t msg_data_length( coap_message_t* msg ) {
	if ( ( msg->data_block != NULL ) && ( msg->data_block->release_list_for_data_size != NULL ) ) {
		return msg->data_block->release_list_for_data_size->data_size;
	}
	PRINTF( "Data block is NULL when trying to get the size of it.\r\n" );
	return 0;// No data or invalid data.
}

/**
 * Add more binary bytes to the payload.
 * The header should already have been updated by calling:
 *
 *    coap_store_response_header()
 *
 *    These functions set the has_payload_bit_flag in msg_out,
 *    but this flag is not set in functions like coap_udp_recv().
 *
 * @param msg_out is the important information for the response.
 * @param data is the data to add to the end of the response.
 * @param size of data.
 * @return WICED_SUCCESS or
 *         WICED_ERROR if input failed validation or
 *         WICED_PACKET_BUFFER_OVERFLOW if inadequate space in msg_out->data.
 *
 * written by Eric Thelin 7 February 2016
 */
wiced_result_t coap_append_response_payload( uint16_t media_type, coap_message_t* msg_out, uint8_t* data, uint16_t size )
{
    if ( ( msg_out == NULL ) || ( data == NULL ) ) {
        PRINTF( "NULL parameter in coap_append_response_payload function.\r\n" );
        return WICED_ERROR;
    }

    if ( size <= 0 ) {// Size 0 is valid, but do nothing.
        return WICED_SUCCESS;
    }

    // Get number of bytes to be added by media type/content format option

    wiced_result_t find_media_type_result = WICED_ERROR;

    int i;
    uint16_t option_value = 0;
    uint16_t payload_index = 0;
    uint16_t preceeding_option_number = 0;
    uint16_t bytes_requrired_for_media_type = 0;
    uint16_t min_required_data_size;

	find_media_type_result = coap_find_numeric_option( CONTENT_FORMAT, msg_out,
    		&option_value, &payload_index, &preceeding_option_number );

    if ( find_media_type_result == WICED_ERROR ) {
    	PRINTF( "Error finding media type in coap_append_response_payload.\r\n");
    	return WICED_ERROR;
    }
    if (  find_media_type_result == WICED_NOT_FOUND ) {// Get bytes required for media type.
    	uint8_t temp[ 5 ];
    	uint16_t temp_preceeding_option_number = preceeding_option_number;// Save old value to use later.

    	bytes_requrired_for_media_type = add_coap_uint_option( CONTENT_FORMAT, media_type, &temp_preceeding_option_number,
    			temp, 5 );
    	if ( bytes_requrired_for_media_type <= 0 ) {
    		PRINTF("Failed to calculate space required for media type option in coap_append_response_payload.\r\n");
    		return WICED_ERROR;
    	}
    }// else - Found - see if statement at the end of this function.

    if ( 0 == msg_out->has_payload_bit_flag ) {// No pre-existing payload on msg.

    	// Check for buffer overrun and resize if possible.
    	min_required_data_size = msg_out->msg_length + 1 + size + bytes_requrired_for_media_type;
        if ( msg_data_length( msg_out ) < min_required_data_size ) {
        	if ( coap_msg_resize( msg_out, min_required_data_size ) != WICED_SUCCESS ) {
                PRINTF( "Buffer overrun in function coap_append_response_payload.\r\n");
                return WICED_PACKET_BUFFER_OVERFLOW;
        	}
        }

        if (  find_media_type_result == WICED_NOT_FOUND ) {
        	uint16_t temp_length = coap_insert_numeric_option( CONTENT_FORMAT, &preceeding_option_number, media_type,
        			payload_index, msg_out );

        	if ( temp_length != bytes_requrired_for_media_type ) {
        		PRINTF( "Wrong option length calculated in coap_append_response_payload.\r\n" );
        	    return WICED_ERROR;
        	}
        }

        msg_out->data_block->data[ msg_out->msg_length ] = PAYLOAD_START;// 0xFF payload marker character.
        msg_out->msg_length++;
        msg_out->has_payload_bit_flag = 1;
    }
    else {// There already is a pre-existing payload.

    	// Check for buffer overrun and resize if possible.
    	min_required_data_size = msg_out->msg_length + size + bytes_requrired_for_media_type;
    	if ( msg_data_length( msg_out ) < min_required_data_size ) {
        	if ( coap_msg_resize( msg_out, min_required_data_size ) != WICED_SUCCESS ) {
				PRINTF( "Buffer overrun in function coap_append_response_payload near end.\r\n");
				return WICED_PACKET_BUFFER_OVERFLOW;
        	}
    	}

        if ( find_media_type_result == WICED_NOT_FOUND ) {
        	// Treat this case as more of a warning than an error.
        	PRINTF( "Unable to identify pre-existing media type for message in coap_append_response_payload.\r\n" );

        	uint16_t temp_length = coap_insert_numeric_option( CONTENT_FORMAT, &preceeding_option_number, media_type,
        			payload_index, msg_out );

        	if ( temp_length != bytes_requrired_for_media_type ) {
        		PRINTF( "Wrong option length calculated in coap_append_response_payload.\r\n" );
        	    return WICED_ERROR;
        	}
        }
    }

    for( i = 0; i<size; i++) {
        msg_out->data_block->data[ msg_out->msg_length + i ] = data[ i ];
    }
    msg_out->msg_length += size;

    // If media type is different from an earlier media type,
    // add content anyway using earlier media type but return an error.
    if ( ( find_media_type_result == WICED_SUCCESS ) && ( option_value != media_type ) ) {
    	PRINTF("Attempt to allocate 2 different media_types to a single network message.\r\n");
    	return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/**
 * Add to the payload of "msg_out" using sprintf.
 *
 * Parameters:
 * media_type is the media type that should be specified with the CONTENT-FORMAT option.
 * msg_out is the message to be sent.
 * format is the sprintf format string
 * ... parameters passed to sprintf.
 *
 * NOTE: If the "msg_out" does not already have a media type specified,
 * then the media type is set. Otherwiase, if the specified "media_type" is different,
 * then go ahead and add the sprintf to the payload, but return WICED_ERROR.
 *
 * written by Eric Thelin 7 February 2016
 */
wiced_result_t coap_append_response_payload_using_printf( uint16_t media_type, coap_message_t* msg_out, char* format,  ... )
{
    if ( ( msg_out == NULL ) || ( format == NULL ) ) {
        PRINTF( "NULL parameter in coap_append_response_payload_using_printf function.\r\n" );
        return WICED_ERROR;
    }
    va_list args;// Arguments from the expanded elipsis "..."

    // Get number of bytes to be added by both the sprintf and media type/content format option

    wiced_result_t find_media_type_result = WICED_ERROR;

    uint16_t option_value = 0;
    uint16_t payload_index = 0;
    uint16_t preceeding_option_number;
    uint16_t bytes_requrired_for_media_type = 0;
    uint16_t min_required_data_size;

	find_media_type_result = coap_find_numeric_option( CONTENT_FORMAT, msg_out,
    		&option_value, &payload_index, &preceeding_option_number );

    if ( find_media_type_result == WICED_ERROR ) {
    	PRINTF( "Error finding media type in coap_append_response_payload_using_printf function.\r\n");
    	return WICED_ERROR;
    }
    if (  find_media_type_result == WICED_NOT_FOUND ) {// Get bytes required for media type.
    	uint8_t temp[ 5 ];
    	uint16_t temp_preceeding_option_number = preceeding_option_number;// Save old value to use later.

    	bytes_requrired_for_media_type = add_coap_uint_option( CONTENT_FORMAT, media_type, &temp_preceeding_option_number,
    			temp, 5 );
    	if ( bytes_requrired_for_media_type <= 0 ) {
    		PRINTF("Failed to calculate space required for media type option in coap_append_response_payload_using_printf.\r\n");
    		return WICED_ERROR;
    	}
    }// else - Found - see if statement at the end of this function.

    va_start ( args, format );// Initialize list of arguments.

    uint16_t size = vsnprintf( NULL, 0, format, args ) + 1;// Get length with room for null terminator

    if (0 == msg_out->has_payload_bit_flag ) {// No pre-existing payload on msg.

    	// Check for buffer overrun and resize if possible.
        min_required_data_size = msg_out->msg_length + 1 + size + bytes_requrired_for_media_type;
    	if ( msg_data_length( msg_out ) < min_required_data_size ) {
        	if ( coap_msg_resize( msg_out, min_required_data_size ) != WICED_SUCCESS ) {
                PRINTF( "Buffer overrun while appending to a COAP message payload.\r\n");

                va_end ( args );// Empty the list
                return WICED_PACKET_BUFFER_OVERFLOW;
        	}
        }
        // Get Location of content format. Insert it.

        if (  find_media_type_result == WICED_NOT_FOUND ) {
        	uint16_t temp_length = coap_insert_numeric_option( CONTENT_FORMAT, &preceeding_option_number, media_type,
        			payload_index, msg_out );
        	if ( temp_length != bytes_requrired_for_media_type ) {
        		PRINTF( "Wrong option length calculated in coap_append_response_payload_using_printf function.\r\n" );

        	    va_end ( args );// Empty the list
        	    return WICED_ERROR;
        	}
        }

        msg_out->data_block->data[ msg_out->msg_length ] = PAYLOAD_START;// 0xFF payload marker character.
        msg_out->msg_length++;
        msg_out->has_payload_bit_flag = 1;
    }
    else {// There already is a pre-existing payload.

    	// Check for buffer overrun and resize if possible.
    	min_required_data_size = msg_out->msg_length + size + bytes_requrired_for_media_type;
    	if ( msg_data_length( msg_out ) < min_required_data_size ) {
        	if ( coap_msg_resize( msg_out, min_required_data_size ) != WICED_SUCCESS ) {
				PRINTF( "Buffer overrun while appending to the end of a COAP message payload.\r\n");

				va_end ( args );// Empty the list
				return WICED_PACKET_BUFFER_OVERFLOW;
        	}
    	}

        if ( find_media_type_result == WICED_NOT_FOUND ) {
        	// Treat this case as more of a warning than an error.
        	PRINTF( "Unable to identify pre-existing media type for message in coap_append_response_payload_using_printf.\r\n" );

        	uint16_t temp_length = coap_insert_numeric_option( CONTENT_FORMAT, &preceeding_option_number, media_type,
        			payload_index, msg_out );

        	if ( temp_length != bytes_requrired_for_media_type ) {
        		PRINTF( "Wrong option length calculated near end in coap_append_response_payload_using_printf.\r\n" );

        		va_end ( args );// Empty the list
        		return WICED_ERROR;
        	}
        }
    }

    vsprintf( (char*) msg_out->data_block->data + msg_out->msg_length, format, args );

    msg_out->msg_length += size - 1;// exclude null terminator

    va_end ( args );// Empty the list

    // If media type is different from an earlier media type,
    // add content anyway using earlier media type but return an error.
    if ( ( find_media_type_result == WICED_SUCCESS ) && ( option_value != media_type ) ) {
    	PRINTF("Attempt to allocate 2 different media_types to a single network message.\r\n");
    	return WICED_ERROR;
    }
    return WICED_SUCCESS;
}


