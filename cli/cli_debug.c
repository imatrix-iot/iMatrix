/*
 * Copyright 2017, Sierra Telecom inc. All Rights Reserved.
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
 * cli_dump.c
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "wiced.h"

#include "interface.h"
#include "cli_debug.h"
#include "messages.h"
#include "../device/config.h"
#include "../storage.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define NO_DEBUG_MSGS   12
const char *debug_flags_description[ NO_DEBUG_MSGS ] =
{
        "General Debugging ",                   // 0x00000001
        "Debugs For BLE",                       // 0x00000002
        "Debugs For Basic CoAP Messaging",      // 0x00000004
        "Debugs For CoAP Xmit",                 // 0x00000008
        "Debugs For CoAP Recv",                 // 0x00000010
        "Debugs For CoAP Support Routines",     // 0x00000020
        "Debugs For Hal",                       // 0x00000040
        "Debugs For iMatrix Upload",            // 0x00000080
        "Debugs For Serial Flash",              // 0x00000100
        "Debugs For Application Start",         // 0x00000200
        "Debugs For Event Driven Entries",      // 0x00000400
        "Debugs for Sample Driven Entries",     // 0x00000800
};

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
 *               Variable Definitions
 ******************************************************/
extern IOT_Device_Config_t device_config;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief
  * @param  None
  * @retval : None
  */
void cli_debug(uint16_t mode)
{
    bool print_flags;
    char *token, *foo;
    uint16_t i;

	/*
	 *	command format debug <on|off>
	 */
    print_flags = false;
	token = strtok(NULL, " " );	// Get argument
	if( token ) {
		if( strcmp( token, "on" ) == 0 ) {
		    device_config.print_debugs = true;
		    imatrix_save_config();
			print_flags = true;
		} else if( strcmp( token, "off" ) == 0 ) {
		    device_config.print_debugs = false;
	        imatrix_save_config();
	} else if( strncmp( token, "?", 1 ) == 0 ) {
	        for( i = 0; i < NO_DEBUG_MSGS; i++ )
	            imx_cli_print( "0x%08lx - %s\r\n", ( (uint32_t) 1 << i ), debug_flags_description[ i ] );
		    print_flags = true;
		} else {
		    if( strncmp( token, "0x", 2 ) == 0 )
		        device_config.log_messages = strtoul( &token[ 2 ], &foo, 16 );
		    else
		        device_config.log_messages = strtoul( token, &foo, 10 );
		    imatrix_save_config();
		}
	} else
	    imx_cli_print( "Invalid option, debug <on|off|?|flags>\r\n" );

	if( print_flags == true ) {
	    imx_cli_print( "Debug: %s, Current debug flags: 0x%08lx\r\n", ( device_config.print_debugs == true ) ? "On" : "Off", device_config.log_messages );
	    for( i = 0; i < NO_DEBUG_MSGS; i++ )
	        if( device_config.log_messages & ( 1 << i ) )
	            imx_cli_print( "0x%08lx - %s\r\n", ( (uint32_t) 1 << i ), debug_flags_description[ i ] );
	}

}
