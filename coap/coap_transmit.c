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
/** @file coam_transmit.c
 *
 *
 *
 */


#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../device/icb_def.h"
#include "coap.h"
#include "que_manager.h"
#include "coap_reliable_udp.h"
#include "sent_message_list.h"
#include "../cli/interface.h"
#include "../cli/messages.h"
#include "../device/config.h"
#include "../wifi/wifi.h"
#include "tcp_transport.h"
#include "coap_transmit.h"
/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_XMIT
    #undef PRINTF
	#define PRINTF(...) if( ( icb.log_messages & DEBUGS_FOR_XMIT ) != 0x00 ) imx_log_printf( __VA_ARGS__)
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
extern message_list_t list_udp_coap_xmit, list_tcp_coap_xmit;
extern wiced_udp_socket_t udp_coap_socket;
extern iMatrix_Control_Block_t icb;
extern tcp_t tcp;
extern wiced_mutex_t udp_xmit_reset_mutex; // variable defined in que_manager.c


/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Coap Transmit process - process the xmit queue
  * @param  None
  * @retval : Never
  */
void coap_transmit( uint16_t process_to_end )
{
	wiced_result_t wiced_result;
    message_t *msg;
    wiced_packet_t   *packet;
    char             *data;
    bool             done;
    uint16_t         i, available_data_length, protocol;
    uint32_t         two_bytes; //temporary variable used to convert from host to network byte order

    uint16_t random_backoff;
    wiced_time_t now;
    wiced_time_get_time( &now );
    protocol = COMM_UDP;
    done = false;
    do {
        if( protocol == COMM_UDP ) {
            msg = (message_t *) list_pop_before( now, &list_udp_coap_xmit );
        } else
            msg = (message_t *) list_pop( &list_tcp_coap_xmit );
        if( msg != NULL ) {
            /* Create the packet */
            if( protocol == COMM_UDP ) {
                // TEMPORARY QUICK ADD-(this is a convenient place to get every response to a multicast, but to assess an appropriate delay
                // you really ought to have an idea of how many hosts will be responding and how much bandwidth each response will require)
                // if there is no time stamp and this is an ipv4 multicast address (most significant 3 bits out of 4 turned on) add delay
                if ( ( msg->coap.next_timestamp == 0 ) &&
                     ( ( msg->coap.my_ip_from_request.ip.v4 & 0xF0000000 ) == 0xE0000000 )) {
                    random_backoff = ms_backoff( 100 );
                    PRINTF("MULTICAST ADDRESS %u.%u.%u.%u backoff: %u mS\r\n",
                            (unsigned char) ( ( msg->coap.my_ip_from_request.ip.v4  >> 24 ) & 0xff ),
                            (unsigned char) ( ( msg->coap.my_ip_from_request.ip.v4  >> 16 ) & 0xff ),
                            (unsigned char) ( ( msg->coap.my_ip_from_request.ip.v4  >>  8 ) & 0xff ),
                            (unsigned char) ( ( msg->coap.my_ip_from_request.ip.v4  >>  0 ) & 0xff ),
                            random_backoff );
                    list_add_at( now + random_backoff, &list_udp_coap_xmit, msg, 0 ); // 0 for retries
                    continue;
                    /*
                            if ( ( strstr( (char*)msg->coap.data_block->data, "awake" ) == NULL)) {
                                imx_printf("Discarding xmit message that does not have awake in it.\r\n" );
                                goto free_msg;
                            }
                    */
                    // END TEMPORARY QUICK ADD
                }

                if ( wiced_packet_create_udp( &udp_coap_socket, sizeof( coap_header_t ) + msg->coap.msg_length, &packet, (uint8_t**) &data, &available_data_length ) != WICED_SUCCESS ) {
                    imx_printf("UDP TX packet creation failed\r\n");
                    icb.ip_stats[ UDP_STATS].packet_creation_failure += 1;
                    goto free_msg;
                } else
                    PRINTF( "About to transmit CoAP message to: %u.%u.%u.%u, %u Bytes \r\n",
                            (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 24 ) & 0xff ),
                            (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 16 ) & 0xff ),
                            (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  8 ) & 0xff ),
                            (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  0 ) & 0xff ),
                            msg->coap.msg_length );
            } else {    // TCP
                if ( wiced_packet_create_tcp( &tcp.socket, sizeof( coap_header_t ) + msg->coap.msg_length, &packet, (uint8_t**) &data, &available_data_length ) != WICED_SUCCESS ) {
                    PRINTF("TCP TX packet creation failed\r\n");
                    icb.ip_stats[ TCP_STATS].packet_creation_failure += 1;
                    goto free_msg;
                } else
                    PRINTF( "About to transmit CoAP message to TCP %u Bytes \r\n", msg->coap.msg_length );

            }
            /*
             * Copy data into the packet. Make sure it will fit first.
             */
            if( sizeof( coap_header_t ) + msg->coap.msg_length > available_data_length ) {
                PRINTF( "UDP TX packet too small for data\r\n" );
                wiced_packet_delete( packet ); /* Delete packet, since the send failed */
                goto free_msg;
            }

            PRINTF( "Message DATA as string:" );
            if ( msg->coap.data_block != NULL ) {
                for( i = 0; i < msg->coap.msg_length; i++ ) {
                    if ( isprint( msg->coap.data_block->data[ i ] ) ) {
                        PRINTF( "%c", msg->coap.data_block->data[ i ] );
                    }
                    else {
                        PRINTF( "*" );
                    }
                }
            }
            PRINTF( "\r\n" );

            data[ 0 ] = msg->coap.header.ver; // first 2 bits are CoAP version
            data[ 0 ] = ( data[ 0 ] << 2 ) | ( msg->coap.header.t ); // next 2 bits are type
            data[ 0 ] = ( data[ 0 ] << 4 ) | ( msg->coap.header.tkl ); // next 4 bits are token length

            data[ 1 ] = msg->coap.header.code;

            //next 2 bytes
            two_bytes = htons( msg->coap.header.id ); // convert id to network byte order
            memmove( &(data[ 2 ]), &two_bytes, 2);

            memmove( data + sizeof( coap_header_t ), msg->coap.data_block->data, msg->coap.msg_length );
            /* Set the end of the data portion */
            wiced_packet_set_data_end( packet, (uint8_t*) data + sizeof( coap_header_t ) + msg->coap.msg_length );

    // If packet is confirmable but was originally sent too long ago don't send - discard packet.

            PRINTF("Sending to: %u.%u.%u.%u data length: %u \r\n",
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 24 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 16 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  8 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  0 ) & 0xff ),
                    msg->coap.msg_length );

            if ( ( msg->coap.data_block != NULL ) && ( msg->coap.data_block->release_list_for_data_size != NULL ) ) {
                PRINTF( "Data block size: %lu\r\n", msg->coap.data_block->release_list_for_data_size->data_size );
            }
            else {
                PRINTF( "NULL data block.\r\n" );
            }

            if( protocol == COMM_UDP ) {
                /* Send the UDP packet */
/*
                imx_printf( "Sending UDP Packet to: %u.%u.%u.%u data length: %u \r\n",
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 24 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >> 16 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  8 ) & 0xff ),
                    (unsigned char) ( ( msg->coap.ip_addr.ip.v4  >>  0 ) & 0xff ),
                    msg->coap.msg_length );
*/
                wiced_result = wiced_udp_send( &udp_coap_socket, &msg->coap.ip_addr, msg->coap.port, packet );
                if ( wiced_result != WICED_SUCCESS ) {
                    imx_printf("UDP packet send failed: %u\r\n", wiced_result );
                    icb.ip_stats[ UDP_STATS].fail_to_send_packet += 1;

                    wiced_packet_delete( packet );  // Delete packet, since the send failed
                    goto free_msg;
                }
                icb.ip_stats[ UDP_STATS].packets_sent += 1;
            } else {
                /* Send the TCP packet */
                wiced_result = wiced_tcp_send_packet( &tcp.socket, packet );
                if ( wiced_result != WICED_SUCCESS ) {
                    PRINTF("TCP packet send failed: %u\r\n", wiced_result );
                    icb.ip_stats[ UDP_STATS].fail_to_send_packet += 1;

                    wiced_packet_delete( packet );  // Delete packet, since the send failed
                    goto free_msg;
                }
                icb.ip_stats[ TCP_STATS].packets_sent += 1;
            }
    // else // SUCCESS
            {
                if ( ( msg->coap.initial_timestamp == 0 ) && ( msg->coap.response_processing_method != IGNORE_RESPONSE ) ) {
                    wiced_time_get_time( &( msg->coap.initial_timestamp ) );
                    expect_response_from( &( msg->coap ) );
                }

                PRINTF("Successfully sent CoAP packet\r\n" );
            }
            /*
             * NOTE : It is not necessary to delete the packet created above, the packet
             *        will be automatically deleted *AFTER* it has been successfully sent
             */

            /*
             * Free up the message
             */
free_msg:
            msg_release( msg );

            if( process_to_end == false ) {
                if( protocol == COMM_UDP )  // Must process at lease 1 message for each queue
                    protocol = COMM_TCP;
                else
                    done = true;
            }
        } else
            if( protocol == COMM_UDP )  // Must process messages for each queue
                protocol = COMM_TCP;
            else
                done = true;
    } while( !done );
}

bool coap_transmit_empty(void)
{
    if( coap_tcp_xmit_empty() && coap_udp_xmit_empty() )
        return true;
    else
        return false;
}

bool coap_tcp_xmit_empty(void)
{
    if( list_size( &list_tcp_coap_xmit ) == 0 )
        return true;
    else
        return false;
}
bool coap_udp_xmit_empty(void)
{
	if( list_size( &list_udp_coap_xmit ) == 0 )
		return true;
	else
		return false;
}
void coap_udp_xmit_reset(uint16_t message_id, wiced_udp_socket_t *socket, wiced_ip_address_t *remote_ip, uint16_t remote_port )
{
    wiced_packet_t   *packet;
    uint8_t          *udp_data;
    uint16_t         available_data_length;
    wiced_result_t result;

    if ( wiced_packet_create_udp( socket, 4, &packet, &udp_data, &available_data_length ) != WICED_SUCCESS )
    {
    	icb.print_msg |= MSG_COAP_XMIT_PACKET_FAILED;
        return;
    }
    if( 4 > available_data_length ) {
    	icb.print_msg |= MSG_COAP_XMIT_SMALL_PACKET;
        wiced_packet_delete( packet ); /* Delete packet, since the send failed */
        return;
    }
    udp_data[ 0 ] = 1; // first 2 bits are CoAP version
    udp_data[ 0 ] = ( udp_data[ 0 ] << 2 ) | ( RESET ); // next 2 bits are type
    udp_data[ 0 ] = ( udp_data[ 0 ] << 4 ) | ( 0 ); // next 4 bits are token length

    udp_data[ 1 ] = EMPTY_MSG; // next byte is the Response Code

    // Use Big endian, network byte order, for message id
    udp_data[ 2 ] = ( message_id >> 8 ) & 0xFF;
    udp_data[ 3 ] = message_id & 0xFF;

    wiced_packet_set_data_end( packet, udp_data + 4 );

    result = wiced_rtos_lock_mutex( &udp_xmit_reset_mutex );
    if( result != WICED_SUCCESS ) {
    	icb.print_msg |= MSG_COAP_XMIT_NO_MUTEX;
    }

    result = wiced_udp_send( socket, remote_ip, remote_port, packet );
    if( result != WICED_SUCCESS ) {
    	icb.print_msg |= MSG_COAP_XMIT_RST_FAILED;
    }

    result = wiced_rtos_unlock_mutex( &udp_xmit_reset_mutex );
    if( result != WICED_SUCCESS ) {
    	icb.print_msg |= MSG_COAP_XMIT_NO_UNLOCK_MUTEX;
    }
}
