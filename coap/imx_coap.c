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

/** @file imx_coap.c
 *
 *  Created on: December 23, 2017
 *      Author: greg.phillips
 *
 *  Define routines to interface to host application coap interface routines.
 */

#include <stdint.h>

#include "wiced.h"

#include "../storage.h"
#include "../device/icb_def.h"

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
extern iMatrix_Control_Block_t icb;

/******************************************************
 *               Function Definitions
 ******************************************************/
/**
 * Test to see if an address is a multicast address in the IPv4 multicast range 224.*.*.* - 239.*.*.*
 * To Do: IPv6 is not implemented yet.
 *
 * @param addr is the ip address to test.
 * @return 1 AKA TRUE if it is a multicast address.
 *         Return 0 AKA WICED_FALSE in all other cases.
 *
 * written by Eric Thelin 12 January 2016
 */
bool imx_is_multicast_ip( wiced_ip_address_t *addr )
{
   if ( addr != NULL ) {
       return ( addr->version == WICED_IPV4 ) && ( ( addr->ip.v4 & 0xF0000000 ) == 0xE0000000 );
   }
   else {
       return false;
   }
}
/**
  * @brief Set the pointer to the coap interface block for host functions
  * @param  No of entries, pointer to block of uris and routines
  * @retval : None
  */
void imx_set_host_coap_interface( uint16_t no_coap_entries, CoAP_entry_t *host_coap_entries )
{
    /*
     * Set up the icb with these vales, no way to really verify, assume user is doing the right thing
     */
    icb.no_host_coap_entries = no_coap_entries;
    icb.coap_entries = host_coap_entries;
}

/**
  * @brief  Determine if this response should be suppressed based on the response
  *         Normally processed multi-cast messages will not send a response. Only error responses will return
  * @param  Address of senders destination, response code
  * @retval : true / false
  */
bool imx_supress_multicast_response( wiced_ip_address_t *addr, uint16_t response )
{
    if( ( imx_is_multicast_ip( addr ) == true ) && ( ( response & MSG_MASK ) == MSG_2_XX ) )
        return true;
    else
        return false;
}
