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
 * significant property damage, injury or death ("High-Risk Product"). By
 * including Sierra's product in a High-Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so, agrees to indemnity Sierra against all liability.
 */
/** @file add_coap_option.c
 *
 *
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "coap.h"
#include "../CoAP_interface/token_string.h"
#include "add_coap_option.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../storage.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_BASIC_MESSAGING
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_BASIC_MESSAGING ) != 0x00 ) imx_printf( __VA_ARGS__)
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
extern IOT_Device_Config_t device_config;   // Defined in device/storage.h

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Insert an option into "msg" starting at "start_array_index" in the payload.
 *
 * Parameters:
 * option_number is the standard CoAP number for this option.
 * preceeding_option_number is the last option number stored in the CoAP header before this one will be inserted.
 * option_value is the number on which the CoAP option operates.
 * start_array_index is the position in "msg->data" where the option should be inserted.
 * msg is a structure containing the payload in the form of "data" and "msg_length" attributes.
 *
 * Returns the number of bytes inserted. If zero, an error occured.
 *
 * written by Eric Thelin 4 February 2016
 */
uint16_t coap_insert_numeric_option( uint16_t option_number, uint16_t *preceeding_option_number, uint16_t option_value,
		uint16_t start_array_index, coap_message_t *msg )
{
	if ( ( preceeding_option_number == NULL ) || ( msg == NULL ) || ( msg->data_block == NULL ) ) {
		PRINTF( "NULL passed to coap_insert_numeric_option function.\r\n" );
		return 0;
	}
	if ( option_number < *preceeding_option_number ) {
		PRINTF( "Out of order option_numbers passed to coap_insert_numeric_option function.\r\n");
		return 0;
	}
	if ( msg->msg_length < start_array_index ) {// Allow start to be the first element after the array.
		PRINTF( "Out of array bounds error in coap_insert_numeric_option function.\r\n");
		return 0;
	}
	uint16_t option_length;

	if ( start_array_index == msg->msg_length ) {// Start at the end of the data.
PRINTF("Adding option to end\r\n");
		option_length = add_coap_uint_option( option_number, option_value, preceeding_option_number, msg->data_block->data + msg->msg_length,
				COAP_PAYLOAD_LENGTH - msg->msg_length );

		msg->msg_length += option_length;

		return option_length;
	}
	else {// There are other options and data after the insertion point for this option.
		uint8_t option_bytes[ 7 ] = { 0, 0, 0, 0, 0, 0, 0 };// up to 5 bytes for header and up to 2 bytes for value
        uint8_t next_delta_bytes[ 3 ] = { 0, 0, 0 };
        uint16_t next_delta = 0;
        uint16_t delta_length = 0, new_delta_length = 0;
        uint16_t *uint16;
        uint16_t size_change = 0;

PRINTF( "Adding option in middle.\r\n" );
        if ( msg->data_block->data[ start_array_index ] != PAYLOAD_START ) {// There is an option following, so change its delta.

			// Get the delta and the number of bytes.

			next_delta = ( 0xF0 & msg->data_block->data[ start_array_index ] ) >> 4;
            delta_length = 1;

			// If option number's delta is not in the first byte, read the extra bytes.
			if ( next_delta > 0xC ) {
				if ( ( next_delta == 0xD ) && ( start_array_index + 1 < COAP_PAYLOAD_LENGTH ) ) {
					next_delta = msg->data_block->data[ start_array_index + 1 ] + 0xD;
					delta_length = 2;
				}
				else if ( ( next_delta == 0xE ) && ( start_array_index + 2 < COAP_PAYLOAD_LENGTH ) ) {
					uint16 = (uint16_t*) ( msg->data_block->data + start_array_index + 1 );
					next_delta = ntohs( *uint16 ) + 0xFF + 0xE;
					delta_length = 3;
				}
				else {
					PRINTF( "Invalid option delta or attempt to read past the end of array.\r\n");
					return 0;
				}
			}
			next_delta = next_delta - ( option_number - *preceeding_option_number );

			// Write new delta in bytes and get new length.

			next_delta_bytes[ 0 ] = 0xF & msg->data_block->data[ start_array_index ]; // Copy nibble representing length of value.

			if ( next_delta <= 0xC ) {// Store delta up to 12 in first byte.
				next_delta_bytes[ 0 ] |= 0xF0 & ( next_delta << 4 );// Move delta to most significant nibble (4 bits).
				new_delta_length = 1;
			}
			else if ( next_delta <= 0xFF + 0xD){// delta > 12 and < 269 must be stored in 1 extra byte.
				next_delta_bytes[ 0 ] |= 0xD0;// First delta nibble = 13.
				next_delta_bytes[ 1 ] =  next_delta - 0xD;// Store delta - 13 in second byte.
				new_delta_length = 2;
			}
			else {// delta must be stored in 2 extra bytes.
				next_delta_bytes[ 0 ] |= 0xE0;// First delta nibble = 14.
				next_delta = htons( next_delta - ( 0xFF + 0xE ) );// Store delta - 269 in next 2 bytes.
				memmove( next_delta_bytes + 1, &next_delta, 2 );
				new_delta_length = 3;
			}
        }
		else { PRINTF( "Inserting before payload marker.\r\n" );}

		option_length = add_coap_uint_option( option_number, option_value, preceeding_option_number, option_bytes, 7 );

		if ( option_length <= 0 ) {// Failed to create option.
			return 0;
		}

		size_change = option_length + new_delta_length - delta_length;
		if ( COAP_PAYLOAD_LENGTH < msg->msg_length + size_change ) {// Buffer overrun error.
			PRINTF("Buffer overrun error in coap_insert_numeric_option function.\r\n");
			return 0;
		}

		memmove( msg->data_block->data + start_array_index + size_change, msg->data_block->data + start_array_index,
				msg->msg_length - start_array_index );

		memmove( msg->data_block->data + start_array_index, option_bytes, option_length );
		memmove( msg->data_block->data + start_array_index + option_length, next_delta_bytes, new_delta_length );

		msg->msg_length += size_change;
PRINTF( "MSG size: %u size_change: %u opt: %u new delta: %u old delta: %u\r\n", msg->msg_length, size_change, option_length, new_delta_length, delta_length );
		return option_length;
	}
}
/**
 * Search any options after the token in the data portion of the "msg" structure.
 * "option_number_in" is the name part in a name value pair.
 * If "option_number_in" is found return the first integer value associated with it and
 * a possible insertion point for another option of this "option_number_in" type.
 * If "option_number_in" is not found still return the insertion point.
 *
 * Parameters:
 * option_number_in is the numeric name of a name-value pair to search for in the list of options in "msg".
 * option_value_out is the integer value to return. If NULL this parameter is ignored.
 * msg is the COAP message structure to search.
 * option_index_out is the index to "msg" data that returns a possible insertion point
 *     for an option of this "option_number_in" type.
 *     If NULL this parameter is ignored.
 * preceeding_option_number_out is the option number of the option before a possible insertion point.
 *     If NULL this parameter is ignored.
 *
 * Returns WICED_SUCCESS, WICED_ERROR or WICED_NOT_FOUND.
 */
wiced_result_t coap_find_numeric_option( uint16_t option_number_in, coap_message_t *msg,
		uint16_t *option_value_out, uint16_t *option_index_out, uint16_t *preceeding_option_number_out )
{
	if ( ( msg == NULL ) || ( msg->data_block == NULL ) ) {
		PRINTF( "NULL sent to coap_find_numeric_option function.\r\n" );
		return WICED_ERROR;
	}
	uint16_t payload_index = msg->header.tkl;
	uint16_t option_value_length = 0;
	uint16_t delta = 0, last_option_number = 0;
	uint16_t *uint16, value_out;
	uint16_t option_is_found = WICED_FALSE;

	// Loop over all options.
	while ( ( payload_index < msg->msg_length ) && ( msg->data_block->data[ payload_index ] != PAYLOAD_START ) ) {

		// Get option number's delta and value length from first byte if possible.
		delta = ( 0xF0 & msg->data_block->data[ payload_index ] ) >> 4;
		option_value_length = 0xF & msg->data_block->data[ payload_index ];
		if ( option_index_out != NULL ) {
		    *option_index_out = payload_index;
		}
		payload_index++;

		// If option number's delta is not in the first byte, read the extra bytes.
		if ( delta > 0xC ) {
			if ( ( delta == 0xD ) && ( payload_index < msg->msg_length ) ) {
				delta = msg->data_block->data[ payload_index ] + 0xD;
				payload_index++;
			}
			else if ( ( delta == 0xE ) && ( payload_index + 1 < msg->msg_length ) ) {
				uint16 = (uint16_t*) ( msg->data_block->data + payload_index );
				delta = ntohs( *uint16 ) + 0xFF + 0xE;
				payload_index += 2;
			}
			else {
				PRINTF( "Invalid option delta or attempt to read past end of array.\r\n");
				return WICED_ERROR;
			}
		}

		// If value size is not in the first byte, read the extra bytes.
		if ( option_value_length > 0xC ) {
			if ( ( option_value_length == 0xD ) && ( payload_index < msg->msg_length ) ) {
				option_value_length = msg->data_block->data[ payload_index ] + 0xD;
				payload_index++;

			}
			else if ( ( option_value_length == 0xE ) && ( payload_index + 1 < msg->msg_length ) ) {
				uint16 = (uint16_t*) ( msg->data_block->data + payload_index );
				option_value_length = ntohs( *uint16 ) + 0xFF + 0xE;
				payload_index += 2;

			}
			else {
				PRINTF( "Invalid option length or attempt to read past end of array. opt len: %.2X indx: %u max len: %u\r\n", option_value_length, payload_index, msg->msg_length );
				return WICED_ERROR;
			}
		}

		// If we have gone past the option we are looking for, we are finished.
		// The index being returned will be the index of the first byte of this option which is set at the top of the loop.
		// The value being returned should have already been set if it was found in the first option with the correct option number.
		if ( option_number_in < delta + last_option_number ) {

			if ( preceeding_option_number_out != NULL ) {
                *preceeding_option_number_out = last_option_number;
			}

			if ( option_is_found == WICED_TRUE ) {
				return WICED_SUCCESS;
			}
			return WICED_NOT_FOUND;
		}

		// If the option is found get the value and
		// if this is the first instance set the option value to be returned.
		if ( option_number_in == delta + last_option_number ) {
			PRINTF("Found %.4X at %u\r\n", option_number_in, payload_index );
			if ( option_value_length == 0 ) {
				value_out = 0;
			}
			else if ( ( option_value_length == 1 ) && ( payload_index < msg->msg_length ) ) {
				value_out = msg->data_block->data[ payload_index ];
			}
			else if ( ( option_value_length == 2 ) && ( payload_index + 1 < msg->msg_length ) ) {
				uint16 = (uint16_t*) ( msg->data_block->data + payload_index );
				value_out = ntohs( *uint16 );
			}
			else {
				PRINTF( "Attempted to treat Non-numeric option value as numeric, or buffer overrun error.\r\n" );
			    return WICED_ERROR;
			}
			if ( ( option_is_found == WICED_FALSE ) && ( option_value_out != NULL ) ) {// return value of first matching entry when finished.
				*option_value_out = value_out;
			}
			option_is_found = WICED_TRUE;
	 	}

		// Get ready to read the next option.
		last_option_number += delta;
		payload_index += option_value_length;
	}

	if ( payload_index > msg->msg_length ) {// The last option value extended past the end of the array.
		PRINTF("Attempted to read option value past end of array. Indx: %u max len %u\r\n", payload_index, msg->msg_length );
		return WICED_ERROR;
	}

	// If we have finished reading all the options,
	// set the returned index to the byte after the end of the last option.
	if ( option_index_out != NULL ) {
	    *option_index_out = payload_index;
	}

	if ( preceeding_option_number_out != NULL ) {
        *preceeding_option_number_out = last_option_number;
	}

	if ( option_is_found == WICED_TRUE ) {
		return WICED_SUCCESS;
	}
	return WICED_NOT_FOUND;
}


/**
 * The first part of the option specifies the option number and the length (number of bytes) of the value.
 * Construct this first part of the option in a 5 byte "header" array as specified in IETF RFC 7252.
 *
 * Parameters:
 * header is the 5 byte output array containing the option number and length of the option's value, but not the value itself.
 * option_number is the standard CoAP number for this option.
 * option_length is the number of bytes in the string or big endian number that will be sent as the value for this option.
 * current_option_number is the last option number stored in the CoAP header before this one will be inserted.
 *
 * Return the number of bytes in "header" that were used. Returning 0 indicates an error occurred.
 *
 * written by Eric Thelin 29 January 2016
 */
uint16_t create_coap_option_header( uint8_t *header, uint16_t option_number, uint16_t option_length, uint16_t current_option_number )
{
	if ( header == NULL ) {
		PRINTF( "NULL value sent to create_coap_option_header function.\r\n" );
		return 0;
	}
    if ( option_number < current_option_number ) {
    	PRINTF( "Attempted to store options out of numeric order.\r\n");
    	return 0;
    }
	uint16_t delta = option_number - current_option_number;
	uint16_t header_length = 1;// Minimum size of option with no value.

	if ( delta <= 0xC ) {// Store delta up to 12 in first byte.
        header[ 0 ] = 0xF0 & ( delta << 4 );// Move delta to most significant nibble (4 bits).
	}
	else if ( delta <= 0xFF + 0xD){// delta > 12 and < 269 must be stored in 1 extra byte.
		header[ 0 ] = 0xD0;// First delta nibble = 13.
		header[ 1 ] =  delta - 0xD;// Store delta - 13 in second byte.
		header_length++;
	}
	else {// delta must be stored in 2 extra bytes.
		header[ 0 ] = 0xE0;// First delta nibble = 14.
		delta = htons( delta - ( 0xFF + 0xE ) );// Store delta - 269 in next 2 bytes.
		memmove( header + 1, &delta, 2 );
		header_length += 2;
	}

	if ( option_length <= 0xC ) {// Store option_length up to 12 in first byte.
        header[ 0 ] |= option_length;// option_length is the least significant nibble.
	}
	else if ( option_length <= 0xFF + 0xD){// option_length > 12 and < 269 must be stored in 1 extra byte.
		header[ 0 ] |= 0xD;// First option_length nibble = 13.
		header[ header_length ] =  option_length - 0xD;// Store option_length - 13 in second byte.
		header_length++;
	}
	else {// option_length must be stored in 2 extra bytes.
		header[ 0 ] |= 0xE;// First option_length nibble = 14.
		option_length = htons( option_length - ( 0xFF + 0xE ) );// Store option_length - 269 in next 2 bytes.
		memmove( header + header_length, &option_length, 2 );
		header_length += 2;
	}
	return header_length;
}

/**
 * Add a single integer valued option of type "option_number" to the beginning of "buffer"
 * using option format specified in IETF RFC 7252.
 *
 * Parameters:
 * option_number is the standard CoAP number for this option.
 * option_value is the number on which the CoAP option operates.
 * current_option_number is the last option number stored in the CoAP header before this one will be inserted.
 * buffer is the part of the CoAP message payload that will get the option.
 * buf_length is the number of bytes in "buffer".
 *
 * Return the number of bytes in "buffer" that were used. Returning 0 indicates an error occurred.
 *
 * written by Eric Thelin 29 January 2016
 */
uint16_t add_coap_uint_option( uint16_t option_number, uint16_t option_value, uint16_t *current_option_number,
		uint8_t *buffer, uint16_t buf_length )
{
	if ( ( buffer == NULL ) || ( current_option_number == NULL ) ) {
		PRINTF( "NULL passed to add_coap_uint_option function.\r\n" );
		return 0;
	}
	uint16_t option_length = 1;
	uint16_t header_length = 0;
	uint8_t header[ 5 ] = { 0, 0, 0, 0, 0 };

	// Calculate minimum size required by option_value.

    if ( option_value <= 0xFF ) {// 8 bit uint
		option_length = 1;
	}
	else {
		option_length = 2;// 16 bit uint
	}

	// Get header for option.

	header_length = create_coap_option_header( header, option_number, option_length, *current_option_number );
	if ( ( header_length <= 0 ) || ( buf_length < header_length + option_length ) ) {
		PRINTF( "Error storing option in COAP message.\r\n" );
		return 0;
	}

	// Copy header into the buffer.

	memmove( buffer, header, header_length );

	// Copy value into buffer.

	if ( option_length == 1 ) {
		buffer[ header_length ] = option_value;
	}
	else if ( option_length == 2 ) {
		option_value = htons( option_value );
		memmove( buffer + header_length, &option_value, 2 );
	}

	*current_option_number = option_number;

	return header_length + option_length;
}

/**
 * Add a single string option of type "option_number" to the beginning of "buffer".
 * using option format specified in IETF RFC 7252.
 *
 * Parameters:
 * option_number is the type of option that will be stored.
 * current_option_number is the last option number stored in the CoAP header before this one will be inserted.
 * option_string is the string containing the value to be assigned to the option.
 * buffer is initially the part of the payload that needs to get the option.
 * buf_length is the number of bytes in "buffer".
 *
 * Return the number of bytes in "buffer" that were used. Returning 0 indicates an error occurred.
 *
 * written by Eric Thelin 31 January 2016
 */
uint16_t add_coap_str_option( uint16_t option_number, uint16_t *current_option_number, char *option_string, uint8_t *buffer, uint16_t buf_length )
{
    if( ( current_option_number == NULL ) || ( option_string == NULL ) || ( buffer == NULL ) ) {
        PRINTF("NULL parameter in add_coap_str_option function.\r\n");
        return 0;
    }
    if( option_number < *current_option_number ) {
        PRINTF("Attempted to store options out of order in COAP message.\r\n");
        return 0;
    }
	uint16_t option_length = strlen( option_string );
	uint16_t header_length = 0;
	uint8_t header[ 5 ] = { 0, 0, 0, 0, 0 };

	// Get header for option.

	header_length = create_coap_option_header( header, option_number, option_length, *current_option_number );
	if ( ( header_length <= 0 ) || ( buf_length < header_length + option_length ) ) {
		PRINTF( "Error storing option in COAP message.\r\n" );
		return 0;
	}

	// Copy header and string into the buffer.

	memmove( buffer, header, header_length );
	memmove( buffer + header_length, option_string, option_length );

    *current_option_number = option_number;

    return header_length + option_length;
}


/**
  * @brief
  * @param  None
  * @retval : None
  */

uint16_t add_coap_uri_option( uint16_t *current_option_number, uint8_t *buffer, char *uri )
{
    uint16_t uri_length, uri_insert_point, foo;

    /*
     * Add the option code and the length
     */
    buffer[ 0 ] = ( ( URI_PATH - *current_option_number ) & 0x0f ) << 4;
    *current_option_number = URI_PATH;

    uri_length = strlen( uri );
    if( uri_length < 13 ) { // Value is 0 - 12 add to this byte
        buffer[ 0 ] |= uri_length;
        uri_insert_point = 1;
    } else if ( uri_length < 269 ) {
        buffer[ 0 ] |= 0x0d;
        buffer[ 1 ] = uri_length - 13;
        uri_insert_point = 2;
    } else {
        buffer[ 0 ] |= 0x0e;
        foo = uri_length - 269;
        buffer[ 1 ] = ( foo & 0xff00 ) >> 8;
        buffer[ 2 ] = ( foo & 0x00ff );
        uri_insert_point = 3;
    }
    strcpy( (char *) &buffer[ uri_insert_point ], uri );
    return ( uri_insert_point + uri_length );
}

/**
 * "str_in" is altered by replacing "separator" characters with NULL terminator charactors
 * and the created list of strings is expanded into a list of coap options in "buffer".
 * All of these options are of type "option_number".
 *
 * @param option_number is the type of option that will be stored.
 * @param separator is the character used to split "str_in" into multiple strings.
 * @param str_in is a list of values for the options.
 * @param current_option_number is initially the last option added to the payload,
*        but becomes the same as "option_number" on a successful exit.
 * @param buffer is initially the part of the payload that needs to get the list of options.
 * @param buf_length is the length of "buffer".
 * @return the number of bytes written to "buffer". This is 0 on error.
 *
 * written by Eric Thelin 15 January 2016
 */
uint16_t add_options_from_string( uint16_t option_number, char separator, char *str_in,
        uint16_t *current_option_number, uint8_t *buffer, uint16_t buf_length )
{
    char *word = str_in;
    char *next_word;
    uint16_t buf_index = 0;
    uint16_t option_length;

    if ( ( str_in == NULL ) || ( current_option_number == NULL ) || ( buffer == NULL ) ) {
        PRINTF( "NULL parameter in add_options_from_string.\r\n");
        return 0;
    }
    if( ( URI_HOST != option_number ) &&
        ( LOCATION_PATH != option_number ) &&
        ( URI_PATH != option_number ) &&
        ( URI_QUERY != option_number ) &&
        ( LOCATION_QUERY != option_number ) &&
        ( PROXY_URI != option_number ) &&
        ( PROXY_SCHEME != option_number ) ) {
        PRINTF("Invalid option number for a list of string options.\r\n");
        return 0;
    }
    if ( 0 == strcmp( str_in, "" ) ) {// Empty string is valid - return 0, don't do anything else.
        return 0;
    }
    // Option list cannot begin or end with the separator character.
    if ( ( separator == str_in[ 0 ] ) || ( separator == str_in[ strlen( str_in ) - 1 ] ) ) {
        PRINTF( "Separator characters are only allowed in the middle of a string in add_options_from_string function.\r\n");
        return 0;
    }
    if( option_number < *current_option_number ) {
        PRINTF("Attempted to store list of options out of order in COAP message.\r\n");
        return 0;
    }
    char split_list[ 2 ] = { separator, '\0' };
    int word_length = get_length_before( split_list, word, strlen( word ) );
    while( word_length < strlen( word ) - 1 ) {// While there is at least one character after the separator.

        word[ word_length ] = '\0';// Terminate string for first word by replacing slash with NULL.
        next_word = word + word_length + 1;// Next word starts after the new NULL.

        // Add word as URI option to message payload.
        option_length = add_coap_str_option( option_number, current_option_number, word,
                &(buffer[ buf_index ]), buf_length - buf_index );

        buf_index += option_length;
        word = next_word;
        word_length = get_length_before( split_list, word, strlen( word ) );
    }
    // The last word should have no slash after it.
    option_length = add_coap_str_option( option_number, current_option_number, word,
            &(buffer[ buf_index ]), buf_length - buf_index );
    return buf_index + option_length;
}
