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

/** @file utility.c
 *
 *  Created on: May 7, 2017
 *      Author: greg.phillips
 */




#include <stdint.h>
#include <stdbool.h>

#include "wiced.h"

#include "../cli/interface.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_DNS_RETRY_COUNT     3
#define DNS_WAIT                5000
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
  * @brief printf_my_mac_address
  * @param  pointer to mac address
  * @retval : None
  */

void print_my_mac_address( wiced_mac_t* mac )
{
    imx_cli_print( "%02X:%02X:%02X:%02X:%02X:%02X", mac->octet[0],
            mac->octet[1],
            mac->octet[2],
            mac->octet[3],
            mac->octet[4],
            mac->octet[5] );
}

bool get_site_ip( char *site, wiced_ip_address_t *address )
{
    uint16_t retry_count;
    wiced_result_t result;

    retry_count = 0;
    while ( retry_count < MAX_DNS_RETRY_COUNT ) {
        result = wiced_hostname_lookup( site, address, DNS_WAIT, WICED_STA_INTERFACE );
        if ( result == WICED_TCPIP_SUCCESS ) {
            return true;
        } else  {
            retry_count++;
        }
    }
    return false;
}

uint64_t htonll(uint64_t n)
{
#if __BYTE_ORDER == __BIG_ENDIAN
//    printf( "Time stamp BE: 0x%08lx%08lx", (uint32_t) ( ( n & 0xFFFFFFFF00000000 ) >> 32), (uint32_t) ( n & 0xFFFFFFFF) );
    return n;
#else
//    printf( "Time stamp SE: 0x%08lx%08lx", htonl( (uint32_t) ( ( n & 0xFFFFFFFF00000000 ) >> 32) ), htonl( (uint32_t) ( n & 0xFFFFFFFF) ) );
    return (((uint64_t)htonl(n)) << 32) + htonl(n >> 32);
#endif
}
