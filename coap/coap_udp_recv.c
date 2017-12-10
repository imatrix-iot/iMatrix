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
 * coap_recv.c
 * Functions used to receive the CoAP protocol
 *
 */


#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "wiced.h"

#include "coap.h"
#include "coap_udp_recv.h"

#include "../networking/get_inbound_destination_ip.h"
#include "coap_transmit.h"
#include "que_manager.h"
#include "../cli/messages.h"
#include "../cli/cli.h"
#include "../device/icb_def.h"


/******************************************************
 *                      Macros
 ******************************************************/
#ifdef PRINT_DEBUGS_FOR_RECV
    #undef PRINTF
	#define PRINTF(...) if( ( device_config.log_messages & DEBUGS_FOR_RECV ) != 0x00 ) st_log_print_status( __VA_ARGS__)
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
extern message_list_t list_udp_coap_recv;
extern wiced_udp_socket_t udp_coap_socket;
extern iMatrix_Control_Block_t icb;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  coap_udp_recv
  * @param  None
  * @retval : None
  */
wiced_udp_socket_callback_t coap_udp_recv(void)
{

	wiced_packet_t*           packet;
    uint8_t*                  rx_data;
    uint16_t                  rx_data_length;
    uint16_t                  available_data_length;
    wiced_ip_address_t        udp_src_ip_addr;
    uint16_t                  udp_src_port;
    wiced_ip_address_t        dest_ip;
	wiced_ip_address_t        my_ip;
    uint16_t i;
    wiced_result_t result;
    message_t *msg;
    uint16_t two_bytes; //
    wiced_utc_time_t now;
    uint16_t coap_data_length = 0;
    uint16_t id = 0;
    wiced_time_get_time( &now );

    result = wiced_udp_receive( &udp_coap_socket, &packet, 0 );
    icb.ip_stats[ UDP_STATS].packets_received += 1;

    if ( ( result != WICED_TCPIP_SUCCESS ) ) {
        icb.ip_stats[ UDP_STATS].packets_received_errors += 1;
		icb.print_msg |= MSG_UDP_REC_ERROR;
		icb.ip_stats[ UDP_STATS].rec_error = result;
    	set_last_packet_time( now );// Should never run out of packets, but if we do just assume the received packet is unicast.
        return ( wiced_udp_socket_callback_t ) result;
    }


    /* Get info about the received UDP packet */
    result = wiced_udp_packet_get_info( packet, &udp_src_ip_addr, &udp_src_port );
    if ( ( result != WICED_TCPIP_SUCCESS ) ) {
		icb.print_msg |= MSG_UDP_GET_INFO_ERROR;
    	set_last_packet_time( now );// Should never run out of packets, but if we do just assume the received packet is unicast.
        return ( wiced_udp_socket_callback_t ) result;
    }

    get_inbound_destination_ip( packet, &dest_ip ); // for multicasting this is different from what it normally is.
    if ( is_multicast_ip( &dest_ip ) )
    {
    	/*
    	 * Was it sent from us? if so ignore
    	 */
    	if ( ( wiced_ip_get_ipv4_address( WICED_STA_INTERFACE, &my_ip ) == WICED_SUCCESS ) && ( my_ip.ip.v4 == udp_src_ip_addr.ip.v4 ) ) {
    		goto recv_cleanup;
    	}
    	icb.ip_stats[ UDP_STATS].packets_multicast_received += 1;
    }
    else {
    	icb.ip_stats[ UDP_STATS].packets_unitcast_received += 1;
    	set_last_packet_time( now );
    }

    /* Extract the received data from the UDP packet */
    wiced_packet_get_data( packet, 0, (uint8_t**) &rx_data, &rx_data_length, &available_data_length );

    if( rx_data_length < 4 ) {   // Not enough data to decode - Dump packet
        goto recv_cleanup;
    }
    memmove(&two_bytes, &(rx_data[ 2 ]), 2); // copy message id to temporary variable in network byte order
    id = ntohs( two_bytes ); // store message id in host byte order//-old code-( (uint16_t ) rx_data[ 2 ] << 8 ) | rx_data[ 3 ];

    coap_data_length = rx_data_length - COAP_HEADER_LENGTH;
    /*
     * Do Not PRINT HERE - if this is needed save to variables and print using print msg logic in do everything - no time to implement at the moment
    PRINTF("RX UDP Packet IP: %u.%u.%u.%u Data 	Len:%u\r\n",
                    (unsigned char) ( ( my_ip.ip.v4  >> 24 ) & 0xff ),
                    (unsigned char) ( ( my_ip.ip.v4  >> 16 ) & 0xff ),
                    (unsigned char) ( ( my_ip.ip.v4  >>  8 ) & 0xff ),
                    (unsigned char) ( ( my_ip.ip.v4  >>  0 ) & 0xff ),
					coap_data_length );
     */


    msg = msg_get( coap_data_length );
    if ( ( msg == NULL ) || ( msg->coap.data_block == NULL) ||
         ( msg->coap.data_block->data == NULL ) ||
	     ( msg->coap.data_block->release_list_for_data_size == NULL ) )
    {
    	if ( msg == NULL ) {
    		icb.print_msg |= MSG_UDP_OUT_OF_MEMORY;
    	}
    	else {
    		icb.print_msg |= MSG_UDP_MEMORY_LEAK;
    	}

        // We Should send back a RST packet as we can not accept any data at this time

    	if ( ! is_multicast_ip( &dest_ip ) ) {
    	    coap_udp_xmit_reset( id, &udp_coap_socket, &udp_src_ip_addr, udp_src_port );
    	}

        goto recv_cleanup;
    }
    /*
     * Copy packet to msg
     */
    msg->coap.ip_addr = udp_src_ip_addr;
    msg->coap.port = udp_src_port;
    msg->coap.my_ip_from_request = dest_ip;
    msg->coap.header.ver = ( rx_data[ 0 ] & 0xC0 ) >> 6;
    msg->coap.header.t = ( rx_data[ 0 ] & 0x30 ) >> 4;
    msg->coap.header.tkl = ( rx_data[ 0 ] & 0x0F );
    msg->coap.header.code = rx_data[ 1 ];
    msg->coap.header.id = id;
    msg->coap.msg_length = coap_data_length;

    for( i = 0; i < coap_data_length; i++ ) {
    	msg->coap.data_block->data[ i ] = rx_data[ i + COAP_HEADER_LENGTH ];
    }
    /*
     * Do Not PRINT HERE - if this is needed save to variables and print using print msg logic in do everything - no time to implement at the moment

    PRINTF( "!!!!!!!! msg size: %u, recv: %u\r\n", msg->coap.data_block->release_list_for_data_size->data_size, coap_data_length );
    */
    /*
     * Add the msg to the CoAP receive list for processing
     */
    list_add( &list_udp_coap_recv, msg );

recv_cleanup:
       /* Delete the received packet, it is no longer needed */
    wiced_packet_delete( packet );

    return ( wiced_udp_socket_callback_t ) WICED_SUCCESS;

}
