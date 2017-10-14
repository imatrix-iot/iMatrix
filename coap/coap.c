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
 * coap.c
 * Functions used to process the CoAP protocol
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "wiced.h"

#include "coap.h"
#include "coap_udp_recv.h"
#include "../CoAP_interface/coap_msg_get_store.h"

/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_COAP_DEFINES
    #undef PRINTF
	#define PRINTF(...) if( ( dcb.log_messages & DEBUGS_FOR_COAP_DEFINES ) != 0x00 ) st_log_print_status( __VA_ARGS__)
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
message_list_t list_free;
message_list_t list_udp_coap_recv;
message_list_t list_udp_coap_xmit;
message_list_t list_tcp_coap_recv;
message_list_t list_tcp_coap_xmit;
wiced_thread_t coap_rx_thread;

wiced_udp_socket_t udp_coap_socket;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  process_coap_msg
  * @param  msg
  * @retval : response, uri
  */
uint16_t process_coap_msg( message_t *msg, CoAP_msg_detail_t *cd )
{
    uint8_t option_code, option_delta, processing_options;
    uint16_t option_length, i;
    char *error_description = "";
    uint16_t response_type = NON_CONFIRMABLE; // Unless the request type is Confirmable - used after bad_message: label

    if ( ( msg == NULL ) || ( cd == NULL ) || ( msg->coap.data_block == NULL ) ) {
        PRINTF( "NULL value sent to process_coap_msg function.\r\n" );
        return COAP_NO_RESPONSE;
    }

    /*
     * Initialize
     */
    option_code = 0;
    option_delta = 0;
    cd->payload_length = 0;
    processing_options = true;
    memset( cd->uri, 0x00, MAX_URI_LENGTH );
    memset( cd->uri_query, 0x00, MAX_URI_LENGTH );
    /*
     * Skip over Tokens
     */
    i = msg->coap.header.tkl;   // Step over any Token bytes
    while( i < msg->coap.msg_length && processing_options == true ) {
        /*
         * Process the Options
         */
//        PRINTF( "Processing Option 0x%02x\r\n", msg->coap.data[ i ] );
        if( msg->coap.data_block->data[ i ] == PAYLOAD_START ) {
            processing_options = false;
            i += 1;     // Now pointing to payload
        } else {
            option_delta = ( ( msg->coap.data_block->data[ i ] & 0xF0 ) >> 4 );
            option_length = msg->coap.data_block->data[ i ] & 0x0F;
            if ( option_delta == OPTION_LENGTH_8BITS ) {
                /*
                 * Check if there is real data in the msg for this extra byte
                 */
                if ( msg->coap.msg_length > i + 1 ) {
                    option_delta = msg->coap.data_block->data[ i + 1 ] + 13;
                    i += 2;
                }
                else {
                    //sprintf( (char * ) msg->coap.data_block->data, "Message Truncated\r\n" );
                    error_description = "Message truncated.";
                    goto bad_message;
                }
            } else if ( option_delta  == OPTION_LENGTH_16BITS ) {
                /*
                 * Check if there is real data in the msg for this extra byte
                 */
                if ( msg->coap.msg_length > i + 2 ) {
                    option_delta = ( msg->coap.data_block->data[ i + 1 ] << 8 ) +  msg->coap.data_block->data[ i + 2 ] + 269;
                    i += 3;
                }
                else {
                    //sprintf( (char * ) msg->coap.data_block->data, "Message Truncated\r\n" );
                    error_description = "Message truncated.";
                    goto bad_message;
                }
            } else if ( option_delta == OPTION_LENGTH_ERROR ) {
                error_description = "Invalid option.";// This packet should probably be dropped or send a reset.-- check spec.
                goto bad_message;
            } else {
                i += 1; // Next byte to process
            }

            option_code += option_delta;

            if ( option_length == OPTION_LENGTH_8BITS ) {
                /*
                 * Check if there is real data in the msg for this extra byte
                 */
                if ( msg->coap.msg_length > i + 1 ) {
                    option_length = msg->coap.data_block->data[ i ] + 13;
                    i += 1;
                }
                else {
                    //sprintf( (char * ) msg->coap.data, "Message Truncated\r\n" );
                    error_description = "Message truncated.";
                    goto bad_message;
                }
            } else if ( option_length  == OPTION_LENGTH_16BITS ) {
                /*
                 * Check if there is real data in the msg for this extra byte
                 */
                if ( msg->coap.msg_length > i + 2 ) {
                    option_length = ( ( msg->coap.data_block->data[ i ] << 8 ) +  msg->coap.data_block->data[ i + 1 ] ) + 269;
                    i += 2;
                }
                else {
                    //sprintf( (char * ) msg->coap.data, "Message Truncated\r\n" );
                    error_description = "Message truncated.";
                    goto bad_message;
                }
            } else if ( option_length == OPTION_LENGTH_ERROR ) {
                error_description = "Invalid option.";// This packet should probably be dropped or send a reset.-- check spec.
                goto bad_message;
            }
            /*
             * Process the option - check if the msg length is valid for this entry
             */
//            PRINTF( "Processing option code: %u, length %u\r\n", option_code, option_length );
            if ( msg->coap.msg_length >= ( i + option_length ) ) {
                switch( option_code ) {
                    case IF_MATCH :
                        PRINTF( "Processing IF Match\r\n" );
                        break;
                    case URI_HOST :
                        PRINTF( "Processing URI Host\r\n" );
                        break;
                    case ETAG :
                        PRINTF( "Processing Etag\r\n" );
                        break;
                    case IF_NONE_MATCH :
                        PRINTF( "Processing IF None Match\r\n" );
                        break;
                    case URI_PORT :
                        PRINTF( "Processing URI Port\r\n" );
                        break;
                    case LOCATION_PATH  :
                        PRINTF( "Processing Location Path\r\n" );
                        break;
                    case URI_PATH :
  //                      PRINTF( "Processing URI Path\r\n" );
                        /*
                         * Add this to the uri Path, make sure this will not exceed the max length
                         */
                        if( ( strlen( cd->uri ) + option_length + 1 ) < MAX_URI_LENGTH ) {
                            strcat( cd->uri, "/" );
                            strncat( cd->uri, (char * ) &msg->coap.data_block->data[ i ], option_length );
                        } else {
                            //sprintf( (char * ) msg->coap.data_block->data, "URI Length exceeded\r\n" );
                            error_description = "URI length exceeded.";
                            goto bad_message;
                        }
                        break;
                    case CONTENT_FORMAT :
 //                       PRINTF( "Processing Content Format\r\n" );
                        break;
                    case MAX_AGE :
                        PRINTF( "Processing Max Age\r\n" );
                        break;
                    case URI_QUERY :
                        PRINTF( "Processing URI Query\r\n" );
                        /*
                         * Add this to the uri Query, make sure this will not exceed the max length
                         */
                        if( ( strlen( cd->uri ) + option_length + 1 ) < MAX_URI_LENGTH ) {
                            if( strlen( cd->uri_query) > 0 )
                                strcat( cd->uri_query, "&" );
                            strncat( cd->uri_query, (char * ) &msg->coap.data_block->data[ i ], option_length );
                        } else {
                            //sprintf( (char * ) msg->coap.data, "URI Length exceeded\r\n" );
                            error_description = "URI length exceeded.";
                            goto bad_message;
                        }

                        break;
                    case ACCEPT :
                        PRINTF( "Processing Accept\r\n" );
                        break;
                    case LOCATION_QUERY :
                        PRINTF( "Processing Location Query\r\n" );
                        break;
                    case BLOCK2 :    // Set the Block size - from Wireshark definition
                        PRINTF( "Processing Block 2\r\n" );
                        cd->blocksize = msg->coap.data_block->data[ i ];
                        break;
                    case BLOCK1 :    // Set the Block size - from Wireshark definition
                        PRINTF( "Processing Block Size\r\n" );
                        cd->blocksize = msg->coap.data_block->data[ i ];
                        break;
                    case PROXY_URI :
                        PRINTF( "Processing Proxy URL\r\n" );
                        break;
                    case PROXY_SCHEME :
                        PRINTF( "Processing Proxy Scheme\r\n" );
                        break;
                    case SIZE1 :
                        PRINTF( "Processing Size1\r\n" );
                        break;
                    default :
                        PRINTF( "Unknown option: %u\r\n", option_code );
                        //sprintf( (char * ) msg->coap.data, "Unrecognized CoAP option: %u\r\n", option_code );
                        error_description = "Unrecognized CoAP option.";
                        goto bad_message;
                }
            }
            else {
                error_description = "Message option truncated.";
                goto bad_message;
            }
            i += option_length;
        }
    }
    /*
     * If processing options = false then we have a payload to process
     */
//    PRINTF( "CoAP Options processing completed, URI: %s\r\n", cd->uri );
//    PRINTF( "                                   URI Query: %s\r\n", cd->uri_query );
    if( processing_options == false ) {
        cd->payload = (char *) &msg->coap.data_block->data[ i ];
        cd->payload_length = msg->coap.msg_length - i;
    } else
        cd->payload = NULL;

    return COAP_MESSAGE_MAY_BE_PROCESSED;

bad_message:
    // default response_type = NON_CONFIRMABLE
	if ( msg->coap.header.t == CONFIRMABLE ) {// Piggy back the response on an Acknowledgment.
		response_type = ACKNOWLEDGEMENT;
	}
    if ( coap_store_response_data( &(msg->coap) , BAD_REQUEST, response_type, error_description, TEXT_MEDIA_TYPE ) != WICED_SUCCESS ) {
    	PRINTF( "Unable to create response to error in coap_delete function.\r\n");
    	return COAP_NO_RESPONSE;
    }
    PRINTF( "Bad Message Detected\r\n" );
    return COAP_SEND_RESPONSE;
}
