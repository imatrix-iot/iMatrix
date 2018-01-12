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
/** @file coap_control_reboot.c
 *
 *
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "../device/icb_def.h"
#include "../cli/interface.h"
#include "../coap/coap.h"
#include "../coap/imx_coap.h"
#include "coap_msg_get_store.h"
#include "../cli/messages.h"
#include "../device/icb_def.h"
#include "../storage.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
    #define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) imx_printf(__VA_ARGS__)
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
extern IOT_Device_Config_t device_config;   // Defined in device/storage.h

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
  * @brief  coap_post_control_reboot
  * @param  CoAP msg pointer, arg
  * @retval : CoAP Response
  */
uint16_t coap_post_control_reboot(coap_message_t *msg, CoAP_msg_detail_t *cd, uint16_t arg)
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
     */
    icb.reboot = true;
    wiced_time_get_time( &icb.reboot_time );
    cli_print( "System will reboot\r\n" );

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
    if ( imx_is_multicast_ip( &( msg->my_ip_from_request ) ) && ( response != CHANGED ) ) {
        return COAP_NO_RESPONSE;
    }
    else {
        return COAP_SEND_RESPONSE;
    }
}

