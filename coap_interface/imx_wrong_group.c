/*
 * $Copyright 2017, Sierra Telecom, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sierra Telecom, Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Sierra Telecom, Inc.$
 */

/** @file
 *
 *  wrong_group.c
 *
 *  Created on: September, 2017
 *      Author: greg.phillips
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"
#include "imatrix.h"

#include "token_string.h"
#include "imx_wrong_group.h"
#include "..\storage.h"

#define FORWARD 1
#define BACKWARD -1
extern IOT_Device_Config_t device_config;
/**
 * This function tries to find a group number in a received COAP message.
 * It looks first in the query string and then in the JSON payload.
 * 
 * @param *msg is a received COAP message that is being processed
 * @return false for (wrong_group)
 *         if the group number matches any of the groups for this device or
 *         if no group is found.
 *         Return TRUE if the group number does not match.
 *
 * written by Eric Thelin 6 January 2016
 */
int imx_wrong_group( CoAP_msg_detail_t *cd )
{
    uint16_t group = 0;

    if ( WICED_SUCCESS != imx_get_uint_from_query_str( "group", &group, cd->uri_query ) ) {
        if ( WICED_SUCCESS != imx_get_group_from_json( cd->payload, cd->payload_length, &group ) ) {
            return false; // no group found means no groups are being excluded -- Group OK
        }
    }
    return ! imx_in_my_groups( group );
}
uint16_t imx_in_my_groups( uint16_t group )
{

    return( ( group == IMX_NO_GROUP_VALUE ) ||
            ( group == 0 ) ||
            ( group == device_config.building_id ) ||
            ( group == device_config.floor_id ) ||
            ( group == device_config.room_id ) ||
            ( group == device_config.group_id ) );
}

/**
 * Extract the "group" number form the JSON object inside the "data" string.
 * Example: data = "{..., "group":5, ...}" returns with *group == 5
 *
 * @param data is a string containing the JASON object
 * @param *group becomes the extracted group number
 * @return WICED_SUCCESS or WICED_ERROR if the group was not found or there was
 *         a JSON format error preventing the object from being fully parsed.
 *
 * NOTE: this routine will not necessarily identify a badly formatted JSON object.
 * As soon as a group number is identified this routine immediately exits without
 * parsing the remainder of the object.
 *
 * written by Eric Thelin 5 January 2016
 */
wiced_result_t imx_get_group_from_json( char *data, uint16_t data_length, uint16_t *group )
{
    if ( ( data == NULL ) || ( group == NULL ) ) {
        return WICED_ERROR;
    }
    *group = 0; // if group is not found return the default group and WICED_ERROR

    uint16_t start = 0, colon = 0, first_letter = 0, last_letter = 0, next_item = 0;

    start = imx_skip_whitespace( FORWARD, data, start, data_length );

    // Require that JSON object starts with '{'
    if ( ( start >= data_length ) || ( data[ start ] != '{' ) ) {
        return WICED_ERROR;
    }

    while ( (start < data_length ) && ( data[ start ] != '}' ) )
    {
        if ( ( data[ start ] == '{') ||
             ( data[ start ] == ',' ) ) {
            start++; // move past the initial brace or comma
        }
        else {
            return WICED_ERROR;
        }

        start = imx_skip_whitespace( FORWARD, data, start, data_length );

        if ( start >= data_length ) {
            return WICED_ERROR;
        }

        // check if name before ":" is "group"

        //last_letter = imx_skip_whitespace( BACKWARD, data, colon - 1, data_length );

        if ( ( 0 == strncmp( "\"group\"", &( data[ start ] ), 7 ) ) ||
             ( 0 == strncmp( "\'group\'", &( data[ start ] ), 7 ) ) ) {
            colon = imx_skip_whitespace( FORWARD, data, start + 7, data_length );

            if ( ( colon >= data_length ) || ( data[ colon ] != ':' ) ) {
                return WICED_ERROR;
            }

            next_item = colon + imx_get_length_before( ",}", data + colon, data_length - colon );

            if ( next_item < data_length ) { // terminating character found
                first_letter = imx_skip_whitespace( FORWARD, data, colon + 1, data_length );
                last_letter = imx_skip_whitespace( BACKWARD, data, next_item - 1, data_length );

                // Removing colon and terminating char should leave uint for group.
                if ( imx_left_str_is_uint( data + first_letter, last_letter - first_letter + 1 ) ) {
                    *group = imx_make_str_uint( data + first_letter, last_letter - first_letter + 1 );
                    return WICED_SUCCESS;
                }
                else {
                    return WICED_ERROR;
                }
            }
            else { // JSON missing terminating character - Malformed JSON
                return WICED_ERROR;
            }
        }
        else { // name is not "group"
            // Skip "name" : ??? where ??? is not a "string", {object} or [array]
            colon =  start + imx_get_length_before( ":", data + start, data_length - start );

            if ( colon >= data_length ) {
                return WICED_ERROR;
            }
            start = colon + imx_get_length_before( "\'\",{}[", data + colon, data_length - colon );

            if ( start >= data_length ) { // terminating character not found
                return WICED_ERROR;
            }

            if ( data[ start ] == '}' ) { // reached end of object without finding "group"
                return WICED_ERROR;
            }

            // Skip "name" : "string"
            if ( ( data[ start ] == '\"' ) || ( data[ start ] == '\'') ) {
                last_letter = imx_end_of_string( data, start, data_length );
                start = imx_skip_whitespace( FORWARD, data, last_letter + 1, data_length );
           }
            // Skip "name" : {object} or [array]
            else if ( ( data[ start ] == '{' ) || ( data[ start ] == '[') ) {
                last_letter = imx_end_of_object( data, start, data_length );
                start = imx_skip_whitespace( FORWARD, data, last_letter + 1, data_length );
           }
       }
    }
    return WICED_ERROR;
}

/**
 * Starting at the "start" index of the "data" string,
 * skip whitespace characters till a non-white space character or
 * an end of the string is reached.
 * The direction maybe forward(+1) or backward(-1).
 *
 * @param direction is +1 for forward, -1 for backward
 * @param data is the string to search
 * @param start is the starting index
 * @param data_length of data
 * @return the index of the non-whitespace character or
 *         if not found or the input couldn't be interpreted,
 *         return data_length, an invalid index.
 *
 * written by Eric Thelin 5 January 2016
 */
uint16_t imx_skip_whitespace( int direction, char *data, uint16_t start, uint16_t data_length )
{
    if ( ( data == NULL ) || ( data_length <= start ) || ( abs( direction ) != 1 ) )
    {
        return data_length;
    }

    while ( ( start < data_length ) &&
            ( ( data[ start ] == ' ' ) || ( data[ start ] == '\t') ||
              ( data[ start ] == '\n') || ( data[ start ] == '\r') ) ) {
        if ( ( start == 0 ) && ( direction == -1 ) ) {
            return data_length;
        }
        start += direction;
    }
    return start;
}

/**
 * Starting with the quote at the "start" index of the "data" string,
 * find a matching quote after it that is not escaped with "\".
 *
 * @param data is the character array to search
 * @param start is the starting index where an initial quote should be
 * @param data_length of data
 * @return the index of quote that matches the one at start or
 *         if not found or the input couldn't be interpreted,
 *         return data_length, an invalid index.
 *
 * Note: This assumes the initial quote is a valid starting quote and does not check
 *       for an escape character before the initial quote.
 *
 * written by Eric Thelin 5 January 2016
 */
uint16_t imx_end_of_string( char *data, uint16_t start, uint16_t data_length )
{
    if ( ( data == NULL ) || ( data_length <= start ) ||
         ( ( data[ start] != '\"') && ( data[ start ] != '\'') ) ) {
        return data_length;
    }
    char quote_mark[ 2 ];
    quote_mark[ 0 ] = data[ start ];
    quote_mark[ 1 ] = '\0';

    start++; // move past the first quote

    uint16_t quote = start + imx_get_length_before( quote_mark, data + start, data_length - start );

    // Skip escaped quotes
    while ( ( quote < data_length ) && ( data[ quote - 1 ] == '\\') )  {

        // check for escaped \.
        start = quote;
        while ( ( start > 0 ) && ( data[ start - 1 ] == '\\' ) ) {
            start--;
        }
        if ( ! ( ( quote - start ) % 2 ) ) { // Even number of \ marks means the quote isn't escaped
            return quote;
        }
        // else Odd number of \ marks means the quote is escaped..

        start = quote + 1; // move past escaped quote
        quote = start + imx_get_length_before( quote_mark, data + start, data_length - start );
    }
    return quote; // == data_length if terminating quote is not found.
}

/**
 * Starting with the open brace-{[ at the "start" index of the "data" string,
 * find a matching close brace-}] after it
 * that is not part of another object or array contained within this one.
 *
 * @param data is the character array to search
 * @param start is the starting index where an open brace should be
 * @param data_length of data
 * @return the index of brace that matches the one at start or
 *         if not found or the input couldn't be interpreted,
 *         return data_length, an invalid index.
 *
 * written by Eric Thelin 5 January 2016
 */
uint16_t imx_end_of_object( char *data, uint16_t start, uint16_t data_length )
{
//    uint16_t data_length = strlen( data );

    if ( ( data == NULL ) || ( data_length <= start ) ||
         ( ( data[ start ] != '{' ) && ( data[ start ] != '[' ) ) ) {
        return data_length;
    }

    char end_brace;
    if ( data[ start ] == '{' ) {
        end_brace = '}';
    }
    else {
        end_brace = ']';
    }

    start++; // move past the opening brace

    uint16_t end = start + imx_get_length_before( "{}[]\'\"", data + start, data_length - start );

    // while another opening brace  or quote is found skip to the end of that object
    while ( ( end < data_length ) &&
            ( ( data[ end ] == '{' ) || ( data[ end ] == '[') ||
              ( data[ end ] == '\'') || ( data[ end ] == '\"' ) ) ) {

        // Remove strings inside this object
        if ( ( data[ end ] == '\'') || ( data[ end ] == '\"' ) ) {
            end = imx_end_of_string( data, end, data_length );
        }
        // Recursively skip over arrays and objects inside this object
        // Recursion terminates when end == data_length or
        // when either type of closing brace is found.
        else {
            end = imx_end_of_object( data, end, data_length );
        }
        if ( end < data_length ) {
            start = end + 1;
            end = start + imx_get_length_before( "{}[]\'\"", data + start, data_length - start );
        }
    }
    if ( ( end < data_length ) && ( data[ end ] == end_brace ) ) {
        return end;
    }
    return data_length; // Terminating brace is not found.
}
