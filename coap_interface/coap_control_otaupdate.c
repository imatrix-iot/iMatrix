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

/** @file
 *
 * coap_control_otaupdate.c
 *
 */


#include <stdint.h>

#include "wiced.h"

#include "../storage.h"
#include "../device/config.h"
#include "../coap/coap.h"
#include "../json/mjson.h"
#include "../coap/add_coap_option.h"
#include "../CoAP_interface/get_uint_from_query_str.h"
#include "../ota_loader/ota_loader.h"
#include "../wifi/wifi.h"
#include "coap_def.h"
#include "coap_msg_get_store.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../storage.h"
#include "coap_control_otaupdate.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
    #define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) imx_log_printf(__VA_ARGS__)
#elif !defined PRINTF
    #define PRINTF(...)
#endif
/******************************************************
 *                    Constants
 ******************************************************/
 #define IGNORE_CHECKSUM32 0xFFFFFFFF
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
  * @brief  coap_post_control_otaupdate
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */
uint16_t coap_post_control_otaupdate(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    char site[ IMX_IMATRIX_SITE_LENGTH ], uri[ IMX_IMATRIX_URI_LENGTH ];
    unsigned int image_no;
    int result;
    int checksum32;
    struct json_attr_t json_attrs[] = {
            {"site",  t_string, .addr.string = site, .len = IMX_IMATRIX_SITE_LENGTH },// Strings default to empty string ""
            {"uri",  t_string, .addr.string = uri, .len = IMX_IMATRIX_URI_LENGTH },
            {"image_no", t_uinteger, .addr.uinteger = (unsigned int *)&image_no, .dflt.uinteger = NO_IMAGE_NO },
			{"cksum", t_integer, .addr.integer = &checksum32, .dflt.integer = (long int)IGNORE_CHECKSUM32 },// initially tried using t_uinteger, but that is only a 31 bit integer.
            {NULL}
    };
    uint16_t response;

    PRINTF( "POST mode - '/control/otaupdate'\r\n");
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

    if( strcmp( site, "" ) == 0 ) {
        PRINTF( "Missing site value in JSON from request.\r\n");
        response = BAD_REQUEST;
        goto bad_data;
    }

    if( strcmp( uri, "" ) == 0 ) {
        PRINTF( "Missing uri value in JSON from request.\r\n");
        response = BAD_REQUEST;
        goto bad_data;
    }

    if( image_no == NO_IMAGE_NO ) {
            response = UNSUPPORTED_CONTENT;
            goto bad_data;
    }

    PRINTF( "Site: %s, uri: %s, cksum: %ld\r\n", site, uri, checksum32);

    /*
     * Got here with maybe GOOD data, do the action and say CHANGED :)
     *
     */

    setup_ota_loader( site, uri, image_no, true, (uint32_t)checksum32 );

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
