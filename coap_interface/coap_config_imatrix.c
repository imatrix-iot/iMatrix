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
#include <stdbool.h>

#include "wiced.h"

#include "storage.h"
#include "../device/config.h"
#include "../coap/coap.h"
#include "../imatrix/registration.h"
#include "../json/mjson.h"
#include "../coap/add_coap_option.h"
#include "../CoAP_interface/get_uint_from_query_str.h"
#include "../wifi/wifi.h"
#include "coap_def.h"
#include "coap_msg_get_store.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"

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
#define NO_SECURITY                 0xFFFF

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
extern IOT_Device_Config_t device_config;   // Defined in device\config.h
extern iMatrix_Control_Block_t icb;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  coap_post_control_otaupdate
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */
uint16_t coap_post_config_imatrix(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    char url[ IMX_IMATRIX_URL_LENGTH ];
    char ssid[ IMX_SSID_LENGTH ], phrase_key[ IMX_WPA2PSK_LENGTH ];
    unsigned int security_type;
    int result;
    struct json_attr_t json_attrs[] = {
            {"ssid",        t_string, .addr.string = ssid, .len = IMX_SSID_LENGTH  },// Defaults to empty string ""
            {"phrase_key",  t_string, .addr.string = phrase_key, .len = IMX_WPA2PSK_LENGTH  },// Defaults to empty string ""
            {"security_type", t_uinteger, .addr.uinteger = &security_type, .dflt.uinteger = NO_SECURITY },
            {"url",  t_string, .addr.string = url, .len = IMX_IMATRIX_URL_LENGTH },
            {NULL}
    };
    uint16_t response;

    PRINTF( "POST mode - '/config/iMatrix'\r\n");
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

    if( strcmp( url, "" ) == 0 ) {
        PRINTF( "Missing uri value in JSON from request.\r\n");
        response = BAD_REQUEST;
        goto bad_data;
    }

    if( strlen( url ) > IMX_IMATRIX_URL_LENGTH ) {
        PRINTF( "uri name too long in JSON from request.\r\n");
        response = BAD_REQUEST;
        goto bad_data;
    }
    if( result || ( strcmp( ssid, "" ) == 0x00 )|| ( security_type == NO_SECURITY ) ) {
        PRINTF( "Invalid JSON object in coap_post_control_securessid function.\r\n");
        response = BAD_REQUEST;
        goto bad_data;
    }


    PRINTF( "SSID: %s, Phrase Key: %s, Security Type: %u, status: %d\r\n", ssid, phrase_key, security_type, result );

    /*
     * Got here with maybe GOOD data, do the action and say CHANGED :)
     *
     */

    set_wifi_st_ssid( ssid, phrase_key, security_type );
    strcpy( device_config.imatrix_public_url, url );
    init_registration();

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

    PRINTF( "Sending response for iMatrix end point\r\n" );

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
    PRINTF( "Sending Bad response for iMatrix end point POST\r\n" );
    return COAP_SEND_RESPONSE;
}
