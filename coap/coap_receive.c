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
 * so agrees to indemnity Sierra against all liability.
 */

/** @file transport.c
 *
 *  Created on: May 21, 2017
 *      Author: greg.phillips
 *
 *      Provide the ability to set up and manage Receiving CoAP Messages
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../device/config.h"
#include "coap.h"
#include "../CoAP_interface/match_uri.h"
#include "../CoAP_interface/coap_msg_get_store.h"
#include "que_manager.h"
#include "coap_receive.h"
#include "../cli/messages.h"
#include "../storage.h"

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
extern message_list_t list_free, list_tcp_coap_xmit, list_tcp_coap_recv, list_udp_coap_xmit, list_udp_coap_recv;
extern CoAP_entry_t CoAP_entries[];// Defined in coap_def.c.
extern IOT_Device_Config_t device_config;   // Defined in device/storage.h

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * returns response
 * @param msg
 * @return
 */
uint16_t handle_request(message_t* msg) {

    CoAP_msg_detail_t cd;
    uint16_t response;
    CoAP_entry_t* matched_entry = NULL;
    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable.

    if ( msg->coap.header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
        response_type = ACKNOWLEDGEMENT;
    }

    response = process_coap_msg( msg, &cd );
    PRINTF( "CoAP message processed. Response: %u, %s\r\n", response, cd.uri );
    /*
     * Processing a GET so unless the msg was bad see if we get that they asked for
     */
//    if( response == BAD_REQUEST ) {
    if( response != COAP_MESSAGE_MAY_BE_PROCESSED ) {//response is COAP_(SEND or NO)_RESPONSE
        PRINTF( "Bad request received!\r\n");
        return response;
    }

    matched_entry = match_uri( cd.uri, CoAP_entries, NO_COAP_ENTRIES );
    if ( matched_entry == NULL ) {// uri was not found in list of known actions

        if ( coap_store_response_header( &(msg->coap), NOT_FOUND, response_type, NULL ) != WICED_SUCCESS )
        {
            imx_printf( "Unable to create response header in handle_request function.\r\n");
            return COAP_NO_RESPONSE;
        }
    }
    else {
        PRINTF( "URI Match Found %s\r\n", matched_entry->node.uri );

        switch( MSG_DETAIL(msg->coap.header.code) ) {
            case GET :
                if ( matched_entry->node.get_function != NULL ) {
                    return (matched_entry->node.get_function)( &msg->coap, &cd, matched_entry->node.arg );
                }
                break;
            case POST :
                if ( matched_entry->node.post_function != NULL ) {
                    return (matched_entry->node.post_function)( &msg->coap, &cd, matched_entry->node.arg );
                }
                break;

            // Currently the Delete & Put methods are not supported with any URI
            //case PUT :
            //case DELETE :
            //default :
        }
        if ( coap_store_response_header( &(msg->coap), METHOD_NOT_ALLOWED, response_type, NULL ) != WICED_SUCCESS )
        {
            imx_printf( "Unable to create response header in function coap_delete.\r\n");
            return COAP_NO_RESPONSE;
        }
    }
    return COAP_SEND_RESPONSE;
}

void handle_response(message_t* msg) {

    CoAP_msg_detail_t cd;
    uint16_t result;

    result = process_coap_msg( msg, &cd );
    PRINTF( "CoAP message processed. Response: %u, %s\r\n", result, cd.uri );
    /*
     * Processing a GET so unless the msg was bad see if we get that they asked for
     */
    if( result != COAP_MESSAGE_MAY_BE_PROCESSED ) {//response is COAP_(SEND or NO)_RESPONSE
        PRINTF( "Bad response received!\r\n");
        return;
    }
    if ( ( MSG_CLASS( msg->coap.header.code ) == CLIENT_ERROR ) ||
              ( MSG_CLASS( msg->coap.header.code ) == SERVER_ERROR ) ) {
        PRINTF( "%u.%u.%u.%u responded to a request with error code: %u.%02u.\r\n", 0xFF & ( msg->coap.ip_addr.ip.v4 >> 24 ),
                0xFF & ( msg->coap.ip_addr.ip.v4 >> 16 ),
                0xFF & ( msg->coap.ip_addr.ip.v4 >> 8 ),
                0xFF & msg->coap.ip_addr.ip.v4,
                MSG_CLASS( msg->coap.header.code ),
                MSG_DETAIL(msg->coap.header.code) );
    }
    else {//it is an invalid response class.
        PRINTF( "Invalid response code in message. - Ignoring." );
    }
}


/**
 * @brief  coap_recv - Process any messages that we have received
 * @param  None
 * @retval : None
 */
void coap_recv(uint16_t process_to_end )
{
    message_t *msg;
    uint16_t confirmable, response, reset_condition, protocol;
    bool done;
/*
 * Look for any data on the receive list and process it. if no data sleep for COAP_RECV_SLEEP
 */
    response = 0;
    confirmable = false;
    reset_condition = false;
    protocol = COMM_UDP;
    done = false;
    do {
        if( protocol == COMM_UDP )
            msg = (message_t *) list_pop( &list_udp_coap_recv );
        else
            msg = ( message_t *) list_pop( &list_tcp_coap_recv );
        if( msg != NULL ) {
            /*
             * We have a message - process it
             */
///*
            PRINTF( "UDP from IP %u.%u.%u.%u:%d Data: ",
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 24 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 16 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  8 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  0 ) & 0xff ),
                    msg->coap.port );

            PRINTF( "\r\nCoap MSG Version: %u, Type: %u, Token Length: %u, Code: %u.%02u, Message ID: %.4x, Length: %u\r\n",
                    msg->coap.header.ver, msg->coap.header.t, msg->coap.header.tkl, MSG_CLASS(msg->coap.header.code), MSG_DETAIL(msg->coap.header.code),
                    msg->coap.header.id, msg->coap.msg_length );
//*/
            /*
             * Process a received CoAP message - based on the Type
             * Do some basic checks to see if this is valid
             */
            if( msg->coap.header.tkl >= 9 ) {
                PRINTF( "Token Length invalid in inbound packet. Message ignored.\r\n" );
                list_release( &list_free, msg );
                continue;
            }
            else {
                switch( msg->coap.header.t ) {
                    case CONFIRMABLE :
                    case NON_CONFIRMABLE :
                        if ( msg->coap.header.t ==  CONFIRMABLE)
                            confirmable = true;
                        else
                            confirmable = false;
                        PRINTF( "Processing Class: %u\r\n", MSG_CLASS(msg->coap.header.code) );
                        if ( REQUEST == MSG_CLASS( msg->coap.header.code ) ) {
                            response = handle_request( msg );
                        }
                        else {
                            handle_response( msg );
                        }
                        break;
                    case ACKNOWLEDGEMENT :
                    case RESET :
                    default :
                        PRINTF( "Message Type %u with Response Code %u.%u Ignored.\r\n", msg->coap.header.t,
                                MSG_CLASS(msg->coap.header.code), MSG_DETAIL(msg->coap.header.code) );
                        response = COAP_NO_RESPONSE;
                        break;
                }
                PRINTF( "Return Value: %u, Response: %x Confirmable: %s, Reset condition: %s\r\n", response, msg->coap.header.code, confirmable ? "true" : "false", reset_condition ? "true" : "false" );
                /*
                 * Generate a response if needed
                 */
                // New approach
                if ( response == COAP_NO_RESPONSE ) {
                    list_release( &list_free, msg );
                } else if ( response == COAP_SEND_RESPONSE ){
                    if( protocol == COMM_UDP )
                        list_add( &list_udp_coap_xmit, msg );
                    else
                        list_add( &list_tcp_coap_xmit, msg );
                }
                // Old approach
                else if ( ( ( response != EMPTY_MSG ) || ( confirmable == true ) ) &&
                        reset_condition == false && response != COAP_NO_RESPONSE ) {
                    /*
                     * Add this message to the xmit que and start a transmit
                     */
                    if( protocol == COMM_UDP )
                        list_add( &list_udp_coap_xmit, msg );
                    else
                        list_add( &list_tcp_coap_xmit, msg );
                } else {    // No confirmation required and NO response after processing
                    /*
                     * Free up the message
                     */
                    list_release( &list_free, msg );
                }
            }
            if( process_to_end == false ) {
                if( protocol == COMM_UDP )  // Must process at lease 1 message for each queue
                    protocol = COMM_TCP;
                else
                    done = true;
            }
        } else  // No more messages
            if( protocol == COMM_UDP )  // Must process messages for each queue
                protocol = COMM_TCP;
            else
                done = true;
    } while( !done );
}
