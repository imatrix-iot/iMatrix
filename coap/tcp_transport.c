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

/** @file tcp_transport.c
 *
 *  Created on: May 21, 2017
 *      Author: greg.phillips
 *
 *      Provide the ability to set up and manage a TCP/TLS link with iMatrix to send and receive CoAP packets
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "coap.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../networking/utility.h"
#include "../time/ck_time.h"
#include "coap_tcp_recv.h"
#include "tcp_transport.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define TCP_RETRY_TIME              ( 60 * 1000 )
#define TCP_CONNECT_TIMEOUT         ( 2 * 1000 )
#define TCP_REGISTRATION_TIMEOUT    ( 5 * 1000 )
#define REGISTRATION_LENGTH         80
/******************************************************
 *                   Enumerations
 ******************************************************/
enum tcp_transport_t {
    TCP_INIT = 0,
    TCP_CLOSE_CONNECTION,
    TCP_CLOSE_SOCKET,
    TCP_DONE,
    TCP_RETRY,
    TCP_REGISTER,
    TCP_GET_RESPONSE,
    TCP_IDLE
};

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
wiced_tcp_socket_callback_t coap_tcp_disconnect( wiced_tcp_socket_t* socket, void* arg );
/******************************************************
 *               Variable Definitions
 ******************************************************/
tcp_t tcp;
wiced_mutex_t tcp_mutex;
extern iMatrix_Control_Block_t icb;
extern IOT_Device_Config_t device_config;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Intialize the TCP Transport facility.
  * @param  None
  * @retval : None
  */
void init_tcp(void)
{
    memset( &tcp, 0x00, sizeof( tcp ) );
    tcp.state = TCP_INIT;   // Start with an Intialization
}
/**
  * @brief  Deinitialixe the TCP Transport facility.
  * @param  None
  * @retval : None
  */
void deinit_tcp(void)
{
    wiced_result_t result;

    tcp.state = TCP_IDLE;   // Do Noting

    if( tcp.tcp_connection_up == true ) {
        result = wiced_tcp_unregister_callbacks( &tcp.socket );
        if( result != WICED_SUCCESS )
            imx_printf( "Unable to unregister TCP callback...\r\n" );

        result = wiced_tcp_delete_socket( &tcp.socket );
        if( result != WICED_SUCCESS )
                imx_printf( "Unable to delete TCP socket...\r\n" );
    }
}
void process_tcp( wiced_time_t current_time )
{
    char *registration, *response;
    uint16_t available_data_length;
    uint16_t retry_count, response_length;
    wiced_packet_t *packet, *rx_packet;
    wiced_result_t result;

    switch( tcp.state ) {
        case TCP_INIT :
            if( ( device_config.AP_setup_mode == true ) || ( icb.wifi_up == false ) ) {
                return;     // We don't do TCP when in setup mode or offline
            }
            return;
            retry_count = 0;
            imx_printf( "Starting TCP Transport Service\r\n" );

            while( retry_count < IMX_MAX_CONNECTION_RETRY_COUNT ) {
                /*
                 * Look up the end point connection for the iMatrix Server
                 */
                if( 1 /* get_site_ip( device_config.imatrix_public_url, &tcp.address ) == true */ ) {
                    tcp.address.ip.v4 = MAKE_IPV4_ADDRESS( 10, 3, 0, 144 );
                    tcp.address.version = WICED_IPV4;
                    imx_printf( "Got IP Address for: %s \r\n", device_config.imatrix_public_url );
                    result = wiced_tcp_create_socket( &tcp.socket, WICED_STA_INTERFACE );
                    if( result != WICED_TCPIP_SUCCESS ) {
                        imx_printf( "Failed to create socket on STA Interface, aborting\r\n" );
                        return;    // We are done
                    }
                    /*  --- Add security details
                    socket->is_root_ca_used = WICED_FALSE;
                    socket->is_client_auth_enabled = WICED_FALSE;

                    if ( security != NULL )
                    {
                        if ( ( result = wiced_tls_init_root_ca_certificates( security->ca_cert, security->ca_cert_len ) ) != WICED_SUCCESS )
                        {
                            goto ERROR_CA_CERT_INIT;
                        }
                        socket->is_root_ca_used = WICED_TRUE;
                        if ( ( security->cert != NULL ) && ( security->key != NULL ) )
                        {
                            result = wiced_tls_init_identity( &socket->tls_identity, (char*) security->key, security->key_len, (const uint8_t*) security->cert, security->cert_len );
                            if ( result != WICED_SUCCESS )
                            {
                                goto ERROR_TLS_INIT;
                            }
                            wiced_tls_init_context( &socket->tls_context, &socket->tls_identity, (const char*) conn->peer_cn );
                            socket->is_client_auth_enabled = WICED_TRUE;
                        }
                        else
                        {
                            wiced_tls_init_context( &socket->tls_context, NULL,  (const char*) conn->peer_cn );

                        }
                        wiced_tcp_enable_tls( &socket->socket, &socket->tls_context );
                    }

                    socket->p_user = p_user;
                    socket->server_ip_address = *server_ip_address;
                    socket->portnumber = portnumber;
                    conn->net_init_ok = WICED_TRUE;
                    */
                    /*
                     * Attempting to connect to
                     */
                    imx_printf( "Connecting iMatrix CoAP Server: %03u.%03u.%03u.%03u ",
                            (unsigned int ) ( ( tcp.address.ip.v4 & 0xff000000 ) >> 24 ),
                            (unsigned int ) ( ( tcp.address.ip.v4 & 0x00ff0000 ) >> 16 ),
                            (unsigned int ) ( ( tcp.address.ip.v4 & 0x0000ff00 ) >> 8 ),
                            (unsigned int ) ( ( tcp.address.ip.v4 & 0x000000ff ) ) );

                    result = wiced_tcp_connect( &tcp.socket, &tcp.address, DEFAULT_COAP_PORT, TCP_CONNECT_TIMEOUT );
                    if( result == WICED_TCPIP_SUCCESS ) {
                        imx_printf( "Successfully Connected to: %s on Port: %u\r\n", device_config.imatrix_public_url, DEFAULT_COAP_PORT );
                        tcp.state = TCP_REGISTER;
                        return;
                    } // else
                    /*
                     * Delete Socket
                     */
                    wiced_tcp_delete_socket( &tcp.socket );
                }
                /*
                 * Failed
                 */
                retry_count++;
            }
            imx_printf( "Failed to establish TCP Connection\r\n" );
            wiced_time_get_time( &tcp.last_attempt );
            tcp.state = TCP_RETRY;
            break;
        case TCP_CLOSE_CONNECTION :
            result = wiced_tcp_disconnect( &tcp.socket );
            if( result != WICED_SUCCESS )
                imx_printf( "TCP Disconnect Failed: %u\r\n", result );
            result = wiced_tcp_delete_socket( &tcp.socket );
            if( result != WICED_SUCCESS )
                imx_printf( "TCP Disconnect Failed: %u\r\n", result );
            /*
             * Retry Connection
             */
            tcp.state = TCP_INIT;
            break;
        case TCP_RETRY :
            if( is_later( current_time, tcp.last_attempt + TCP_RETRY_TIME ) )
                tcp.state = TCP_INIT;
            break;
        case TCP_REGISTER :
            /*
             * Commence a registration
             */
            /* Create the TCP packet. Memory for the tx_data is automatically allocated */
            result = wiced_packet_create_tcp(&tcp.socket, REGISTRATION_LENGTH, &packet, (uint8_t**)&registration, &available_data_length);
            if( ( result != WICED_SUCCESS ) || ( available_data_length < REGISTRATION_LENGTH ) ) {
                imx_printf("TCP packet creation failed, in registration\r\n" );
                tcp.state = TCP_INIT;
            }
            /*
             * Create Entry
             */
            sprintf( registration, "{ \"serial_number\" : \"%s\", \"manufactuer id\" : \"0x%08lX\" }", device_config.device_serial_number, device_config.manufactuer_id );
            /*
             * Set the end of the data portion
             */
            wiced_packet_set_data_end( packet, (uint8_t*) registration + strlen( registration ) );
            /*
             * Send it
             */
            result = wiced_tcp_send_packet( &tcp.socket, packet );
            if( result != WICED_SUCCESS ) {
                imx_printf("TCP packet creation failed, in registration\r\n" );
                tcp.state = TCP_CLOSE_CONNECTION;
            } else {
                /*
                 * Get Response
                 */
                tcp.last_attempt = current_time;
                tcp.state = TCP_GET_RESPONSE;
            }
            break;
        case TCP_GET_RESPONSE :
            result = wiced_tcp_receive( &tcp.socket, &rx_packet, 0 );
            if( result == WICED_SUCCESS ) {
                imx_printf( "TCP response received: " );
                /*
                 * Get the contents of the received packet
                 */
                wiced_packet_get_data(rx_packet, 0, (uint8_t**)&response, &response_length, &available_data_length);
                 /*
                  * Null terminate the received string
                  */
                response[ response_length ] = '\x0';
                imx_printf( "Response from CoAP Server: %s\r\n", response );
                /*
                 * Delete the packet
                 */
                wiced_packet_delete( rx_packet );
                /*
                 * Process packets with this callback
                 */
                result = wiced_tcp_register_callbacks( &tcp.socket, NULL, (wiced_tcp_socket_callback_t) &coap_tcp_recv, (wiced_tcp_socket_callback_t) &coap_tcp_disconnect, &tcp.socket );
                if( result == WICED_TCPIP_SUCCESS ) {
                    tcp.tcp_connection_up = true;
                    tcp.state = TCP_IDLE;
                    icb.comm_mode = COMM_TCP;
                    return;
                } else
                    tcp.state = TCP_CLOSE_CONNECTION;
            } else {
                if( is_later( current_time, tcp.last_attempt + TCP_REGISTRATION_TIMEOUT ) ) {
                    imx_printf( "Timed out waiting for response from CoAP Server\r\n" );
                    tcp.state = TCP_CLOSE_CONNECTION;
                }
            }
            break;
        case TCP_IDLE :
            break;
    }
}

wiced_tcp_socket_callback_t coap_tcp_disconnect( wiced_tcp_socket_t* socket, void* arg )
{
    UNUSED_PARAMETER( arg );
    wiced_result_t result;

    result = wiced_rtos_lock_mutex( &tcp_mutex );
    if( result != WICED_SUCCESS ) {
        imx_printf( "TCP Unable to lock mutex...\r\n" );
    }

    if( socket == &tcp.socket )     // Is it for the main TCP stream (Other is Telnet etc.
        tcp.state = TCP_CLOSE_CONNECTION;

    result = wiced_rtos_unlock_mutex( &tcp_mutex );
    if( result != WICED_SUCCESS ) {
        imx_printf( "TCP Unable to un lock mutex...\r\n" );
    }
    return WICED_SUCCESS;
}
