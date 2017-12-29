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
/** @file imatrix_get_ip.c
 *
 *
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include "wiced.h"

#include "../storage.h"
#include "../cli/interface.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "../time/ck_time.h"
/******************************************************
 *                      Macros
 ******************************************************/
//#define USE_FIXED_SERVER
//#define USE_GREG_TEST_SERVER

#define DNS_UPDATE_RATE	( 10 * ( 60 * 1000 ) )	// Update every 10 Minutes
#define MAX_DNS_FAILURE	10
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
extern IOT_Device_Config_t device_config;	// Defined in device\config.h
extern iMatrix_Control_Block_t icb;
static wiced_time_t dns_update_time;
static uint16_t local_dns_failure_count = 0;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief
  * @param  None
  * @retval : None
  */
uint16_t get_imatrix_ip_address( wiced_time_t current_time )
{
	bool result;

	result = false;
	if( ( imx_is_later( current_time, dns_update_time + DNS_UPDATE_RATE ) == true ) || icb.dns_lookup == false ) {
		dns_update_time = current_time;
#ifdef USE_FIXED_SERVER

		icb.imatrix_public_ip_address.ip.v4 = MAKE_IPV4_ADDRESS( 193, 93, 13, 64 );
		icb.imatrix_public_ip_address.version = WICED_IPV4;
		result = true;

#else
#ifdef USE_GREG_TEST_SERVER

		        icb.imatrix_public_ip_address.ip.v4 = MAKE_IPV4_ADDRESS( 10, 3, 0, 250 );
		        icb.imatrix_public_ip_address.version = WICED_IPV4;
		        result = true;
#else
		if ( wiced_hostname_lookup( device_config.imatrix_public_url, &icb.imatrix_public_ip_address, 3 * SECONDS, WICED_STA_INTERFACE ) == WICED_SUCCESS ) {
			imx_printf( "iMatrix DNS IP Address lookup successful " );
			result = true;
		} else {
			imx_printf( "iMatrix IP Address DNS lookup failed for: %s ", device_config.imatrix_public_url );
			icb.dns_failure_count += 1;
			local_dns_failure_count += 1;
			if( local_dns_failure_count >= MAX_DNS_FAILURE ) {
			    local_dns_failure_count = 0;    // Restart at 0
				/*
				 * Restart the Wi Fi to see if we can resolve this
				 */
				icb.wifi_up = false;	// This will cause it to teardown and retry
			}
		}
#endif
#endif
		imx_printf( "set to iMatrix Server: %03u.%03u.%03u.%03u\r\n",
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0xff000000 ) >> 24 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x00ff0000 ) >> 16 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x0000ff00 ) >> 8 ),
			(unsigned int ) ( ( icb.imatrix_public_ip_address.ip.v4 & 0x000000ff ) ) );
		icb.dns_lookup = result;
		return result;
	} else
		return true;
}
