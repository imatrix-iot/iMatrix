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
/** @file coap_securessid.c
 *
 *
 *  Created on: February, 2015
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/icb_def.h"
#include "../device/config.h"
#include "../coap/coap.h"
#include "../json/mjson.h"
#include "../coap/add_coap_option.h"
#include "../CoAP_interface/get_uint_from_query_str.h"
#include "../wifi/wifi.h"
#include "coap_def.h"
#include "coap_msg_get_store.h"
#include "coap_control_securessid.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
	#define PRINTF(...) if( ( icb.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) imx_log_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif
/******************************************************
 *                    Constants
 ******************************************************/
#define SSID_OUTPUT_BUFFER_LENGTH	256
#define NO_SECURITY					0xFFFF

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
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  coap_get_control_securessid
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */

/**
 * Process a control/securessid get request
 *
 * @param msg is initially the request, but is returned as the response.
 * @param cd contains the query string extracted from msg.
 * @param arg is not used.
 * @return COAP_SEND_RESPONSE unless the request was a multicast that does not need a response.
 *         In that case return COAP_NO_RESPONSE.
 *
 * written by Greg Philips 2017-02-15
 */
uint16_t coap_get_control_securessid(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );
    char buffer[ SSID_OUTPUT_BUFFER_LENGTH ];

    if ( ( msg == NULL ) || ( cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_get_control_securessid function.\r\n" );
        return COAP_NO_RESPONSE;
    }
	uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    imx_printf( "Get Configuration - '/control/securessid'\r\n");
    PRINTF( "URI Query: %s\r\n", cd->uri_query );

    memset( buffer, 0x00, SSID_OUTPUT_BUFFER_LENGTH );

    sprintf( buffer, "{ \"ssid\" : \"%s\", \"phrase_key\" : \"%s\", \"security_type\" : %lu }",
    		strcmp( device_config.st_ssid, "" ) == 0x00 ? "null" : device_config.st_ssid,
    		strcmp( device_config.st_wpa, "" ) == 0x00 ? "null" : device_config.st_wpa, device_config.st_security_mode );
    imx_printf( "%s\r\n", buffer );
    if ( coap_store_response_data( msg, CONTENT, response_type, buffer, JSON_MEDIA_TYPE ) != WICED_SUCCESS ) {
			PRINTF( "Failed to create response.\r\n" );
			return COAP_NO_RESPONSE;
    }

    return COAP_SEND_RESPONSE;
}
/**
 * Set the fixture securessid register at offset to "value
 * using "id" and "value" from the request's JSON payload.
 *
 * @param msg is initially the request, but is returned as the response.
 * @param cd contains the query string extracted from msg.
 * @param arg is not used.
 * @return COAP_SEND_RESPONSE or
 *         COAP_NO_RESPONSE if the inbound message is so messed up it can't be interpreted or
 *         COAP_NO_RESPONSE when the inbound message was a multicast and the response is anything other than CHANGED.
 *
 * written by Greg Philips 2014-02-15
 */
uint16_t coap_post_control_securessid(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );

    char ssid[ IMX_SSID_LENGTH ], phrase_key[ IMX_WPA2PSK_LENGTH ];
    unsigned int security_type;
    int result;// Error code returned from JSON read.
    struct json_attr_t json_attrs[] = {
		{"ssid",  		t_string, .addr.string = ssid, .len = IMX_SSID_LENGTH  },// Defaults to empty string ""
		{"phrase_key", 	t_string, .addr.string = phrase_key, .len = IMX_WPA2PSK_LENGTH  },// Defaults to empty string ""
		{"security_type", t_uinteger, .addr.uinteger = &security_type, .dflt.uinteger = NO_SECURITY },

        {NULL}
    };

    if ( ( msg == NULL ) || ( cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_post_control_param.\r\n" );
        return COAP_NO_RESPONSE;
    }

    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }
    // if no response code is assigned assume a server error
    uint16_t response_code = INTERNAL_SERVER_ERROR;

    PRINTF( "POST Configuration - '/control/securessid'\r\n");
    /*
     * Process the passed URI Query
     */
    if( strlen( cd->uri_query ) > 0 ) {
        PRINTF( "Query string sent to coap_post_control_securessid instead of JSON object.\r\n");
        response_code = BAD_REQUEST;
        goto create_response_and_exit;
    }
    PRINTF( "URI Query: %s\r\n", cd->uri_query );
    PRINTF( "URI Payload: %s\r\n", cd->payload );

    result = json_read_object( cd->payload, json_attrs, NULL );

    PRINTF( "SSID: %s, Phrase Key: %s, Security Type: %u, status: %d\r\n", ssid, phrase_key, security_type, result );

    if( result || ( strcmp( ssid, "" ) == 0x00 )|| ( strcmp( phrase_key, "" ) == 0x00 ) || ( security_type == NO_SECURITY ) ) {
        PRINTF( "Invalid JSON object in coap_post_control_securessid function.\r\n");
        response_code = BAD_REQUEST;
        goto create_response_and_exit;
    }
    set_wifi_st_ssid( ssid, phrase_key, security_type );
    response_code = CHANGED;

create_response_and_exit:
    // if inbound message is so badly messed up a response can't be created properly, don't bother.
	if( WICED_SUCCESS != coap_store_response_header( msg, response_code, response_type, NULL ) ) {
	    return COAP_NO_RESPONSE;
	}

	// Suppress all responses to multicast except CHANGED.
	if ( is_multicast_ip( &( msg->my_ip_from_request ) ) && ( response_code != CHANGED ) ) {
	    return COAP_NO_RESPONSE;
	}
	else {
	    return COAP_SEND_RESPONSE;
	}
}
