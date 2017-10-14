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
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../coap/coap.h"
#include "../json/mjson.h"
#include "../coap/add_coap_option.h"
#include "../CoAP_interface/get_uint_from_query_str.h"
#include "../wifi/wifi.h"
#include "coap_def.h"
#include "coap_msg_get_store.h"
#include "coap_control_securessid.h"

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
#define CPUID_LENGTH                        26
#define PRODUCT_ID_SN_TEMPLATE		"{ \"organization-id\" : \"%08lu\", \"product-name\" : \"%s\", \"product-id\" : \"%08lu\", \"serial-number\" : \"%s\", \"cpuid\" : \"0x%08lX%08lX%08lX\", \"capabilities\" : \"%08lu\" }"
#define PRODUCT_ID_SN_BUFFER_LENGTH	( sizeof( PRODUCT_ID_SN_TEMPLATE ) + IMX_PRODUCT_NAME_LENGTH + IMX_PRODUCT_ID_LENGTH + IMX_DEVICE_SERIAL_NUMBER_LENGTH + CPUID_LENGTH + 1 )
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
  * @brief  coap_get_config_imatrix
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */

/**
 * Process a control/iMatrix get request
 *
 * @param msg is initially the request, but is returned as the response.
 * @param cd contains the query string extracted from msg.
 * @param arg is not used.
 * @return COAP_SEND_RESPONSE unless the request was a multicast that does not need a response.
 *         In that case return COAP_NO_RESPONSE.
 *
 * written by Greg Philips 2016-02-15
 */
uint16_t coap_get_control_imatrix(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );
    char buffer[ PRODUCT_ID_SN_BUFFER_LENGTH ];

    if ( ( msg == NULL ) || ( cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_get_config_imatrix function.\r\n" );
        return COAP_NO_RESPONSE;
    }
	uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    PRINTF( "Get Configuration - '/control/imatrix'\r\n");
    PRINTF( "URI Query: %s\r\n", cd->uri_query );

    memset( buffer, 0x00, PRODUCT_ID_SN_BUFFER_LENGTH );

    sprintf( buffer, PRODUCT_ID_SN_TEMPLATE, device_config.organization_id, device_config.product_name,
            device_config.product_id, device_config.device_serial_number, device_config.sn.serial1, device_config.sn.serial2, device_config.sn.serial3, device_config.product_capabilities );
    if ( coap_store_response_data( msg, CONTENT, response_type, buffer, JSON_MEDIA_TYPE ) != WICED_SUCCESS ) {
			PRINTF( "Failed to create response.\r\n" );
			return COAP_NO_RESPONSE;
    }

    return COAP_SEND_RESPONSE;
}

/**
  * @brief  coap_post_control_reboot
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */
uint16_t coap_post_control_imatrix(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
{
    UNUSED_PARAMETER( arg );

    if ( ( msg == NULL ) || ( cd == NULL ) ) {
        PRINTF( "NULL value sent to coap_post_control_reboot function.\r\n" );
        return COAP_NO_RESPONSE;
    }
    uint16_t response;

    PRINTF( "POST mode - '/control/reboot'\r\n");
    /*
     * Process the passed URI Query
     */
    if( strlen( cd->uri_query ) > 0 ) {
        PRINTF( "Query string sent to coap_post_control_demand instead of JSON object.\r\n");
        response = BAD_REQUEST;     // No URI Query supported
        goto bad_data;
    }

    PRINTF( "URI Query: %s\r\n", cd->uri_query );
    PRINTF( "URI Payload: %s\r\n", cd->payload );

    /*
     * Got here with GOOD data, do the action and say CHANGED :)
     *
     * Assume that the system has been programmed with valid credentials - drop Wi Fi and come up in station mode
     */
    icb.wifi_up = false;
    device_config.AP_setup_mode = false;
    imatrix_save_config();
    print_status( "System will attempt connection to iMatrix\r\n" );

    response = CHANGED;

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

    // Suppress all responses to multicast except CHANGED.
    if ( is_multicast_ip( &( msg->my_ip_from_request ) ) && ( response != CHANGED ) ) {
        return COAP_NO_RESPONSE;
    }
    else {
        return COAP_SEND_RESPONSE;
    }
}

