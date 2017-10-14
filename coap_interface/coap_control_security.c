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
 *  Created on: December, 2016
 *      Author: greg.phillips
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "wiced.h"
#include "wiced_framework.h"

#include "../storage.h"
#include "../coap/coap.h"
#include "../cli/interface.h"
#include "../device/cert_defs.h"
#include "../device/cert_util.h"
#include "../device/icb_def.h"
#include "../json/mjson.h"
#include "../coap/add_coap_option.h"
#include "../coAP_interface/get_uint_from_query_str.h"
#include "../wifi/wifi.h"
#include "../device/cert_defs.h"
#include "../device/cert_util.h"
#include "coap_def.h"
#include "coap_msg_get_store.h"
#include "coap_control_security.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
	#define PRINTF(...) if( ( icb.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) st_log_print_status(__VA_ARGS__)
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
extern iMatrix_Control_Block_t icb;
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
  * @brief  coap_get_control_security
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */

/**
 * Get the appropriate security certificate
 *
 * @param msg is initially the request, but is returned as the response.
 * @param cd contains the query string extracted from msg.
 * @param arg is not used.
 * @return COAP_SEND_RESPONSE unless the request was a multicast that does not need a response.
 *         In that case return COAP_NO_RESPONSE.
 *
 * written by Greg Philips 2016-12-02
 */
uint16_t coap_get_control_security(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );
    uint16_t cert_length, cert_type;
    uint8_t buffer[ MAX_CERT_LENGTH ];

    cert_length = 0;
    if ( ( msg == NULL ) || ( cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_get_control_securessid function.\r\n" );
        return COAP_NO_RESPONSE;
    }
	uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    uint16_t payload_index = 0;// Used to print debugging information only.

    if ( WICED_SUCCESS != coap_store_response_header( msg, CONTENT, response_type, &payload_index ) ) {
        PRINTF( "Failed to create response.\r\n" );
        return COAP_NO_RESPONSE;
    }

    PRINTF( "Get Configuration - '/control/security'\r\n");
    PRINTF( "URI Query: %s\r\n", cd->uri_query );

    if ( WICED_SUCCESS != get_uint_from_query_str( "cert_type", &cert_type, cd->uri_query ) ) {// Require ID
        if( coap_store_response_header( msg, BAD_REQUEST, response_type, NULL )  != WICED_SUCCESS ) {
    		PRINTF( "Failed to create response.\r\n" );
        }
        return COAP_NO_RESPONSE;
    }
/*
 * cert_type is ok
 */
    update_cert_pointers();

    switch( cert_type ) {
    	case CA_ROOT_CERTIFICATE :
    		/*
    		 * Verify that the certificate is valid for processing.
    		 */
    		if( verify_certificate( icb.root_certificate, IS_CERTIFICATE ) ) {
    	    	/*
    	    	 * Convert Certificate to Binary -
    	    	 */
    			cert_length = convert_certificate_to_bin( buffer, MAX_CERT_LENGTH, (uint8_t*) ( (uint32_t) (icb.root_certificate) + BEGIN_CERT_LENGTH ),
    					(uint32_t) ( strlen( (char *) icb.root_certificate) - BEGIN_END_CERT_LENGTH ), IS_CERTIFICATE );
    			if( cert_length == 0 ) {
    				print_status( "Certificate is 0 length\r\n" );
    			}
    		} else {
    			print_status( "Certificate is invalid\r\n" );
    			return COAP_NO_RESPONSE;
    		}
    		break;
    	case WIFI_CERTIFICATE_KEY :
    		if( verify_certificate( icb.wifi_8021X_key, IS_KEY ) ) {
    	    	/*
    	    	 * Convert Certificate to Binary -
    	    	 */
    			cert_length = convert_certificate_to_bin( buffer, MAX_CERT_LENGTH, (uint8_t*) ( icb.wifi_8021X_key + BEGIN_KEY_LENGTH ),
    					(uint32_t) ( strlen( (char *) icb.wifi_8021X_key) - BEGIN_END_KEY_LENGTH ), IS_KEY );
    			if( cert_length == 0 ) {
    				print_status( "Certificate is 0 length\r\n" );
    			}
    		} else {
    			print_status( "Certificate is invalid\r\n" );
    			return COAP_NO_RESPONSE;
    		}
    		break;
    	case WIFI_CERTIFICATE :
    		if( verify_certificate( icb.wifi_8021X_certificate, IS_CERTIFICATE ) ) {
    	    	/*
    	    	 * Convert Certificate to Binary -
    	    	 */
    			cert_length = convert_certificate_to_bin( buffer, MAX_CERT_LENGTH, (uint8_t*) ( icb.wifi_8021X_certificate + BEGIN_CERT_LENGTH ),
    					(uint32_t) ( strlen( (char *) icb.wifi_8021X_certificate) - BEGIN_END_CERT_LENGTH ), IS_CERTIFICATE );
    			if( cert_length == 0 ) {
    				print_status( "Certificate is 0 length\r\n" );
    			}
    		} else {
    			print_status( "Certificate is invalid\r\n" );
    			return COAP_NO_RESPONSE;
    		}
    		break;
    	case DTLS_CERTIFICATE :
    		if( verify_certificate( icb.dtls_certificate, IS_CERTIFICATE  ) ) {
    	    	/*
    	    	 * Convert Certificate to Binary -
    	    	 */
    			cert_length = convert_certificate_to_bin( buffer, MAX_CERT_LENGTH, (uint8_t*) ( icb.dtls_certificate + BEGIN_CERT_LENGTH ),
    					(uint32_t) ( strlen( (char *) icb.dtls_certificate ) - BEGIN_END_CERT_LENGTH ), IS_CERTIFICATE );
    			if( cert_length == 0 ) {
    				print_status( "Certificate is 0 length\r\n" );
    			}
    		} else {
    			print_status( "Certificate is invalid\r\n" );
    			return COAP_NO_RESPONSE;
    		}
    		break;
    	default :
    	    if( coap_store_response_header( msg, BAD_REQUEST, response_type, NULL )  != WICED_SUCCESS ) {
    			PRINTF( "Failed to create response.\r\n" );
    	    }
    	    return COAP_NO_RESPONSE;
    	    break;
    }
    /*
     * Add the binary data to the pay load
     */
    if ( WICED_SUCCESS != coap_append_response_payload( BINARY_MEDIA_TYPE, msg, (uint8_t*)buffer, cert_length ) ) {// Copy 2 bytes.
        PRINTF( "Error copying data into a message buffer.\r\n" );
        coap_store_response_header( msg, INTERNAL_SERVER_ERROR, response_type, NULL );
    }

    return COAP_SEND_RESPONSE;
}
/**
 *
 * Save the certificate to the appropriate location in the DCT
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
uint16_t coap_post_control_security(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
	wiced_result_t result;
    UNUSED_PARAMETER( arg );

	uint8_t new_certificate[ MAX_CERT_LENGTH ];


    if ( ( msg == NULL ) || ( cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_post_control_security.\r\n" );
        return COAP_NO_RESPONSE;
    }

    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }
    // if no response code is assigned assume a server error
    uint16_t response_code = INTERNAL_SERVER_ERROR;

    PRINTF( "POST Configuration - '/control/security'\r\n");
    /*
     * Process the passed URI Query
     */
    if( ( strlen( cd->uri_query ) > 0 ) || cd->payload_length < MIN_CERT_LENGTH ) {
        PRINTF( "Query string sent to coap_post_control_securessid instead of Binary object.\r\n");
        response_code = BAD_REQUEST;
        goto create_response_and_exit;
    }
    PRINTF( "URI Query: %s\r\n", cd->uri_query );
    PRINTF( "URI Payload: %s\r\n", cd->payload );

    /*
     * Select which type of certificate it is
     */
    result = WICED_SUCCESS;
    update_cert_pointers();
    print_status( "Processing new credentials: %u Bytes\r\n", cd->payload_length - 1 );

    /*
     * Note the +2 for the write includes 2 null bytes put on in the bin -> certs function
     */
    switch( cd->payload[ 0 ] ) {
    	case CA_ROOT_CERTIFICATE :
    		convert_bin_to_certificate( (uint8_t *)&cd->payload[ 1 ], (uint32_t) cd->payload_length - 1, new_certificate, (uint32_t) MAX_CERT_LENGTH, IS_CERTIFICATE );
    		print_status( "New Root certificate: \r\n%s\r\n", new_certificate );
    		/*
    		 * Save to DCT
    		 */
    	    result = wiced_dct_write( (const void*) new_certificate, DCT_SECURITY_SECTION, OFFSETOF(platform_dct_security_t, certificate) + ROOT_CA_OFFSET, strlen( (char*) new_certificate ) + 2 );
    		break;
    	case WIFI_CERTIFICATE :
    		convert_bin_to_certificate( (uint8_t *)&cd->payload[ 1 ], (uint32_t) cd->payload_length - 1, new_certificate, (uint32_t) MAX_CERT_LENGTH, IS_CERTIFICATE );
    		print_status( "New WiFi certificate: \r\n%s\r\n", new_certificate );
    		/*
    		 * Save to DCT
    		 */
    		result = wiced_dct_write( (const void*) new_certificate, DCT_SECURITY_SECTION, OFFSETOF(platform_dct_security_t, certificate), strlen( (char *) new_certificate ) + 2 );
    		break;
    	case WIFI_CERTIFICATE_KEY :
    		convert_bin_to_certificate( (uint8_t *)&cd->payload[ 1 ], (uint32_t) cd->payload_length - 1, new_certificate, (uint32_t) MAX_CERT_LENGTH, IS_KEY );
    		print_status( "New WiFi Key : \r\n%s\r\n", new_certificate );
    		/*
    		 * Save to DCT
    		 */
    		result = wiced_dct_write( (const void*) new_certificate, DCT_SECURITY_SECTION, OFFSETOF(platform_dct_security_t, private_key), strlen( (char *) new_certificate ) + 2 );
    		break;
    	case DTLS_CERTIFICATE :
    		print_status( "Ignoring DTLS Certificate - Using WiFi instead\r\n" );
			break;
    	default:
            response_code = BAD_REQUEST;
            goto create_response_and_exit;
    		break;	// Just for completeness
    }

    if( result != WICED_SUCCESS )
    	print_status( "Failed to write credentials to DCT\r\n" );

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
