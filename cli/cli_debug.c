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
#include "../device/icb_def.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
const char *debug_flags_description[] =
{
        "General Debugging ",                   // 0x00000001
        "Debugs For BLE",                       // 0x00000002
        "Debugs For Basic CoAP Messaging",      // 0x00000004
        "Debugs For CoAP Xmit",                 // 0x00000008
        "Debugs For CoAP Recv",                 // 0x00000010
        "Debugs For CoAP Support Routines",     // 0x00000020
        "Debugs For Hal",                       // 0x00000040
        "Debugs For History",                   // 0x00000080
        "Debugs For Serial Flash",              // 0x00000100
        "Debugs For Application Start",         // 0x00000200
        "Debugs For Log messages to iMatrix",   // 0x00000400
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
extern iMatrix_Control_Block_t icb;

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
			icb.print_debugs = true;
			print_flags = true;
		} else if( strcmp( token, "off" ) == 0 )
            icb.print_debugs = false;
		else if( strncmp( token, "?", 1 ) == 0 ) {
	        for( i = 0; i < 11; i++ )
	            cli_print( "0x%08lx - %s\r\n", ( (uint32_t) 1 << i ), debug_flags_description[ i ] );
		    print_flags = true;
		} else {
		    if( strncmp( token, "0x", 2 ) == 0 )
		        icb.log_messages = strtoul( &token[ 2 ], &foo, 16 );
		    else
		        icb.log_messages = strtoul( token, &foo, 10 );

		    if( ( icb.log_messages & DEBUGS_LOG_TO_IMATRIX ) != 0x00 ) {
		        cli_print( "Turning of CoAP Debug messages - recursive calls would result\r\n" );
		        icb.log_messages &= ~( (uint32_t) ( DEBUGS_FOR_BASIC_MESSAGING | DEBUGS_FOR_XMIT | DEBUGS_FOR_RECV | DEBUGS_FOR_COAP_DEFINES ) );
		    }
		}
	} else
	    cli_print( "Invalid option, debug <on|off|?|flags>\r\n" );

	if( print_flags == true ) {
	    cli_print( "Debug: %s, Current debug flags: 0x%08lx\r\n", ( icb.print_debugs == true ) ? "On" : "Off", icb.log_messages );
	    for( i = 0; i < 11; i++ )
	        if( icb.log_messages & ( 1 << i ) )
	            cli_print( "0x%08lx - %s\r\n", ( (uint32_t) 1 << i ), debug_flags_description[ i ] );
	}

}
