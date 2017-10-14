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
 *	get_inbound_destination_ip.c
 *
 */

#include <stdint.h>
#include "nx_api.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"

#include "wiced.h"


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

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
 * This function uses the ThreadX NetX Duo: NX_PACKET structure to retrieve the landing IP address of an inbound packet.
 * IPv6 logic has not been tested yet.
 * NOTE: The landing IP address is primarily needed to determine if the inbound packet was a multicast packet or not.
 *
 * NOTE2: the logic for IPv6 is quite different from that for IPv4. Since most IPv6 devices have multiple IP addresses,
 * it is normal to need access to which destination IP was in the Request. ThreadX already has a packet member that
 * probably gives you access to that address. I'm using the source address for a return packet. I could not find an
 * IPv4 equivalent, so I extract the IPv4 value from the header. This is a hack. This ThreadX packet structure is not an
 * exact representation of the inbound packet and some of the fields like the inbound IP version were incorrect. This packet
 * structure is intended to be used for a response, but it contains a lot of the information from the received packet.
 * In testing the IP header, the 4 bytes starting with the 17th byte contained the required IPv4 address.
 *
 * written by Eric Thelin 15 May 2015
 */
void get_inbound_destination_ip( wiced_packet_t* packet, wiced_ip_address_t* my_ip ) // need wiced_ip(NX_PACKET)
{
    int n; //for loop counter

    my_ip->version = WICED_IPV6;
    if ( packet->nx_packet_ip_version == NX_IP_VERSION_V4 ) {
        my_ip->version = WICED_IPV4;
        my_ip->ip.v4 = *((uint32_t *) &(packet->nx_packet_ip_header[16]));
    }
    else { // assume IPv6 - default -- untested code
        for ( n = 0; n < 4; n++) {
            my_ip->ip.v6[n] = packet->nx_packet_ipv6_src_addr[n]; // src addr for sending a packet back
        }
    }
}
