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
 * get_uint_from_querystring.c
 *
 *  Created on: Jan 9, 2016
 *      Author: eric
 */

#include <stdbool.h>

#include "wiced.h"
#include "token_string.h"
#include "imx_get_uint_from_query_str.h"
#include "../cli/interface.h"

/**
 * Extract the number from the "query_str" string that is assigned to "name".
 * Example: query_str = "...&[name]=5&..." returns with *value == 5
 *
 * @param name is part of a name=value pair
 * @param value is the number being retrieved
 * @param query_str is the query string from the COAP packet
 * @return WICED_SUCCESS or WICED_ERROR if no "name" was present or the query string couldn't be interpreted.
 *         In the latter case "value" is returned as 0.
 *
 * written by Eric Thelin 9 January 2016
 */
wiced_result_t imx_get_uint_from_query_str( char* name, uint16_t *value, char* query_str )
{
    int a = 0;
    int option_end = 0;
    int value_end = 0;
    char* short_query;
    char* value_and_more;

    *value = 0; // If no name is found return 0 for value.

    if ( ( name == NULL ) || ( query_str == NULL ) ) {
        imx_printf( "error: NULL input to get_uint_from_query_str function.\r\n" );
        return WICED_ERROR;
    }

    while ( a < strlen( query_str ) ) {

        short_query = &(query_str[a]); //get the right side of query starting at index a

        option_end = imx_get_length_before( "&=", short_query, strlen( short_query ) );

        if ( option_end <= 0 ) {// No token in front of & or =
            imx_printf( "error: infinite while loop in function get_group_from_query_str.\n\r" );
            return WICED_ERROR;
        }
        if ( short_query[ option_end ] != '=' ) {// not an option=value pair--invalid
            imx_printf( "error: query string contains something other than option=value pairs\n\r" );
            return WICED_ERROR;
        }
        //option name OK, need value
        value_end = imx_get_length_before( "&", short_query, strlen( short_query ) );

        if ( value_end <= option_end + 1 ) { // no value after option=
            imx_printf( "error: query string has an option= nothing!\n\r" );
            return WICED_ERROR;
        }
        // else there is some kind of value

        if ( imx_left_str_equals( name, short_query, option_end ) ) {

            value_and_more = &( query_str[a + option_end + 1 ] );

            value_end = imx_get_length_before( "&", value_and_more,
                strlen( value_and_more ) );

            if ( imx_left_str_is_uint( value_and_more, value_end ) ) {
                *value = imx_make_str_uint( value_and_more, value_end );
                return WICED_SUCCESS;
            }
            else {
                imx_printf( "error: non-numeric expression for %s in query string\n\r", name );
                return WICED_ERROR;
            }
        }

        a = a + value_end + 1;//next option starts after "&" in option=value&
    }
    return WICED_ERROR;
}

/**
 * Extract the number from the "query_str" string that is assigned to "name".
 * Example: query_str = "...&[name]=5&..." returns with *value == 5
 *
 * @param name is part of a name=value pair
 * @param value is the number being retrieved
 * @param query_str is the query string from the COAP packet
 * @return WICED_SUCCESS or WICED_ERROR if no "name" was present or the query string couldn't be interpreted.
 *         In the latter case "value" is returned as 0.
 *
 * written by Eric Thelin 9 January 2016
 * updated by Greg Phillips 14 April 2017 to support unsigned long
 */
wiced_result_t imx_get_uint32_from_query_str( char* name, uint32_t *value, char* query_str )
{
    int a = 0;
    int option_end = 0;
    int value_end = 0;
    char* short_query;
    char* value_and_more;

    *value = 0; // If no name is found return 0 for value.

    if ( ( name == NULL ) || ( query_str == NULL ) ) {
        imx_printf( "error: NULL input to get_uint_from_query_str function.\r\n" );
        return WICED_ERROR;
    }

    while ( a < strlen( query_str ) ) {

        short_query = &(query_str[a]); //get the right side of query starting at index a

        option_end = imx_get_length_before( "&=", short_query, strlen( short_query ) );

        if ( option_end <= 0 ) {// No token in front of & or =
            imx_printf( "error: infinite while loop in function get_group_from_query_str.\n\r" );
            return WICED_ERROR;
        }
        if ( short_query[ option_end ] != '=' ) {// not an option=value pair--invalid
            imx_printf( "error: query string contains something other than option=value pairs\n\r" );
            return WICED_ERROR;
        }
        //option name OK, need value
        value_end = imx_get_length_before( "&", short_query, strlen( short_query ) );

        if ( value_end <= option_end + 1 ) { // no value after option=
            imx_printf( "error: query string has an option= nothing!\n\r" );
            return WICED_ERROR;
        }
        // else there is some kind of value

        if ( imx_left_str_equals( name, short_query, option_end ) ) {

            value_and_more = &( query_str[a + option_end + 1 ] );

            value_end = imx_get_length_before( "&", value_and_more,
                strlen( value_and_more ) );

            if ( imx_left_str_is_uint( value_and_more, value_end ) ) {
                *value = strtoul( value_and_more, (char**) ( (uint32_t) value_and_more + value_end ), 10 );
                return WICED_SUCCESS;
            }
            else {
                imx_printf( "error: non-numeric expression for %s in query string\n\r", name );
                return WICED_ERROR;
            }
        }

        a = a + value_end + 1;//next option starts after "&" in option=value&
    }
    return WICED_ERROR;
}
