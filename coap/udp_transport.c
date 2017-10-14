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

/** @file udp_transport.c
 *
 *  Created on: May 15, 2017
 *      Author: greg.phillips
 *
 *      Provide the ability to set up and manage a UDP/DTLS link with iMatrix to send and receive CoAP packets
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../wifi/wifi.h"
#include "coap.h"
#include "que_manager.h"
#include "coap_udp_recv.h"
#include "udp_transport.h"
/******************************************************
 *                      Macros
 ******************************************************/

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
extern wiced_udp_socket_t udp_coap_socket;
extern IOT_Device_Config_t device_config;
extern iMatrix_Control_Block_t icb;
extern message_list_t list_udp_coap_recv;
extern message_list_t list_udp_coap_xmit;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief  Intialize UDP
  * @param  interface to initialize
  * @retval : success / Failure
  */
bool init_udp( wiced_interface_t interface )
{
    wiced_ip_address_t multicast_address = {.ip.v4 = MAKE_IPV4_ADDRESS( 224, 0,  1,  187 ), .version = WICED_IPV4 };

    if( wiced_udp_create_socket( &udp_coap_socket, DEFAULT_COAP_PORT, interface ) != WICED_SUCCESS ) {
        print_status( "Unable to create CoAP UDP Socket...\r\n" );
        goto deregister_link_callbacks_if_needed_and_fail;
    }


    if( wiced_udp_register_callbacks( &udp_coap_socket, (wiced_udp_socket_callback_t) coap_udp_recv, NULL ) != WICED_SUCCESS ) {
         print_status( "Unable to setup CoAP UDP callback...\r\n" );
         goto delete_socket_and_fail;
     }
    /*
     * Use an AP for all communications
     * Join the CoAP Multicast group
     */

    if( wiced_multicast_join(interface, &multicast_address ) != WICED_SUCCESS ) {
        print_status( "Unable to Join Multicast...\r\n" );
        goto unregister_udp_callbacks_and_fail;
    }

    /*
     * Join the CoAP Multicast group
     */

    if( wiced_multicast_join(interface, &multicast_address ) != WICED_SUCCESS ) {
        print_status( "Unable to Join Multicast...\r\n" );
        goto unregister_udp_callbacks_and_fail;
    }
    /*
     * UDP Gets initialized first so if it comes up set it to the comm_mode to use
     */
    icb.comm_mode = COMM_UDP;
    return true;
    /*
     * If an error occurred attempt to gracefully tear down the network and return false.
     */

unregister_udp_callbacks_and_fail:
    wiced_udp_unregister_callbacks( &udp_coap_socket );// Always succeeds.

delete_socket_and_fail:
    if ( wiced_udp_delete_socket( &udp_coap_socket ) != WICED_SUCCESS ) {
        goto reboot_on_error;
    }

deregister_link_callbacks_if_needed_and_fail:
    if( device_config.AP_setup_mode == false ) {
        wiced_network_deregister_link_callback( link_up, link_down, interface );// Always returns success.
    }

    return false;

    /*
     * If the tear down fails, REBOOT.
     */
reboot_on_error:
    wiced_rtos_delay_milliseconds( 1000 );// Wait a second to ensure printf is properly flushed.
    wiced_framework_reboot();
    return false;   // Should never get here
}
/**
  * @brief  de Intialize UDP
  * @param  Interface to de intialize
  * @retval : None
  */
void deinit_udp( wiced_interface_t interface )
 {
    // Clear messages from xmit and receive lists.

    list_release_all( &list_udp_coap_xmit );
    list_release_all( &list_udp_coap_recv );

    wiced_ip_address_t multicast_address = {.ip.v4 = MAKE_IPV4_ADDRESS( 224, 0,  1,  187 ), .version = WICED_IPV4 };

    wiced_multicast_leave( interface, &multicast_address );

    wiced_udp_unregister_callbacks( &udp_coap_socket );

    if ( wiced_udp_delete_socket( &udp_coap_socket ) != WICED_SUCCESS ) {
        goto reboot_on_error;
    }
    return;

reboot_on_error:
    wiced_rtos_delay_milliseconds( 1000 );// Wait a second to ensure printf is properly flushed.
    wiced_framework_reboot();

 }
