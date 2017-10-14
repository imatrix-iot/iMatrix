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
/** @file coap_control_getlatest.c
 *
 *	CoAP Message to start an OTA update to the latest image revision
 *
 */


#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../cli/interface.h"
#include "../coap/coap.h"
#include "../json/mjson.h"
#include "coap_msg_get_store.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
    #define PRINTF(...) if( ( fc.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) st_log_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif

/******************************************************
 *                    Constants
 ******************************************************/
#define IMAGE_NAME_LENGTH 15

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

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
  * @brief  coap_post_control_otagetlatest
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */
uint16_t coap_post_control_otagetlatest(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    uint16_t response;
    char image[ IMAGE_NAME_LENGTH ];
    int16_t result;

    struct json_attr_t json_attrs[] = {
			{"image",  t_string, .addr.string = image, .len = IMAGE_NAME_LENGTH  },// Defaults to empty string ""
			{NULL}
    };

    PRINTF( "POST mode - '/control/otagetlatest'\r\n");
    /*
     * Process the passed URI Query
     */
    if( strlen( cd->uri_query ) > 0 ) {
        response = BAD_REQUEST;     // No URI Query supported
        goto bad_data;
    }
    PRINTF( "URI Query: %s\r\n", cd->uri_query );
    PRINTF( "URI Payload: %s\r\n", cd->payload );

    result = json_read_object( cd->payload, json_attrs, NULL );

    if( result ) {
    	PRINTF( "Result: %u\r\n", result );
    	response = BAD_REQUEST;
    	goto bad_data;
    }
    /*
     * Got here with maybe GOOD data, do the action and say CHANGED :)
     *
     */
    /*
     * Do not reenter OTA if already in progress
     */

	print_status( "Attempting to get latest firmware\r\n" );
	/*
	if( strcmp( image, "master" ) == 0x00 ) {
		setup_get_latest_version( OTA_IMAGE_MASTER, SITE );
		cli_print( "OTA Get Latest 'master' set to run\r\n" );
	} else if( strcmp( image, "sflash" ) == 0x00 ) {
		setup_get_latest_version( OTA_IMAGE_SFLASH, SITE );
		cli_print( "OTA Get Latest 'sflash' set to run\r\n" );
	} else if( strcmp( image, "beta_master" ) == 0x00 ) {
		setup_get_latest_version( OTA_IMAGE_BETA_MASTER, SITE );
		cli_print( "OTA Get Latest beta 'master' set to run\r\n" );
	} else if( strcmp( image, "beta_slave" ) == 0x00 ) {
	} else if( strcmp( image, "beta_sflash" ) == 0x00 ) {
		setup_get_latest_version( OTA_IMAGE_BETA_SFLASH, SITE );
		cli_print( "OTA Get Latest beta 'sflash' set to run\r\n" );
	} else {
    	response = BAD_REQUEST;
    	goto bad_data;
	}
	*/
    if( msg->header.t == CONFIRMABLE ) {
        if( coap_store_response_header( msg, CHANGED, ACKNOWLEDGEMENT, NULL )  != WICED_SUCCESS ) {
            return COAP_NO_RESPONSE;
        }
    } else {
        if( coap_store_response_header( msg, CHANGED, NON_CONFIRMABLE, NULL ) != WICED_SUCCESS ) {
            return COAP_NO_RESPONSE;
        }
    }
    return COAP_SEND_RESPONSE;

    PRINTF( "Sending response for ota update request\r\n" );

    return COAP_NO_RESPONSE;

bad_data:

    if( msg->header.t == CONFIRMABLE ) {
        if( coap_store_response_header( msg, response, ACKNOWLEDGEMENT, NULL )  != WICED_SUCCESS ) {
            return COAP_NO_RESPONSE;
        }
    } else {
        if( coap_store_response_header( msg, response, NON_CONFIRMABLE, NULL ) != WICED_SUCCESS ) {
            return COAP_NO_RESPONSE;
        }
    }
    PRINTF( "Sending Bad response for ota update POST\r\n" );
    return COAP_SEND_RESPONSE;
}

