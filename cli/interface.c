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
 *
 *  Created on: September, 2016
 *      Author: greg.phillips
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include "wiced.h"
#include "platform.h"

#include "../storage.h"
#include "../device/config.h"
#include "../device/icb_def.h"
#include "interface.h"
#include "telnetd.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define SPRINTF_BUFFER_LENGTH	128

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
uint16_t active_device = CONSOLE_OUTPUT;
extern iMatrix_Control_Block_t icb;
extern IOT_Device_Config_t device_config;
extern const char *debug_flags_description[];
/******************************************************
 *               Function Definitions
 ******************************************************/
bool verify_cmd( void )
{
	char ch;

	cli_print( "\r\nAre u sure (y/n): " );

	while( !imx_get_ch( &ch ) )
		;
	cli_print( "%c\r\n", ch );
	if( toupper( (uint8_t )ch ) == 'Y' ) {
		cli_print( "Command Accepted\r\n" );
		return true;
	} else {
	    cli_print( "Command Ignored\r\n" );
		return false;
	}
}

bool imx_get_ch( char *ch )
{
	uint32_t expected_data_size;
	wiced_result_t result;

	if( active_device == CONSOLE_OUTPUT ) {
		expected_data_size = 1;
		result =  wiced_uart_receive_bytes( STDIO_UART, ch, &expected_data_size, (uint32_t) 0 );
		if( result == WICED_SUCCESS )
			return true;
		else
			return false;
	} else if( active_device == TELNET_OUTPUT ) {
		telnetd();	// Make sure we process any characters as we may be in a hard loop somewhere else
		return( telnetd_getch( ch ) );
	} else
		return false;
}

void imx_printf( char *format, ... )
{
    /*
     * Check if output for status messages is enabled.
     *
     * This is only valid if this system is not using AT commands or in set up mode
     */
    if( (( device_config.no_at_controls + device_config.no_at_sensors ) == 0 ) || ( device_config.AT_verbose == IMX_AT_VERBOSE_STANDARD_STATUS )
            || ( device_config.AP_setup_mode == true ) || ( icb.print_debugs == true ) ) {
        va_list args;// Arguments from the expanded elipsis "..."
        va_start( args, format );// Initialize list of arguments.
        vprintf( format, args );
        fflush(stdout);
    }

}


void cli_print( char *format, ... )
{
	char buffer[ SPRINTF_BUFFER_LENGTH ];
	va_list args;// Arguments from the expanded elipsis "..."

    va_start( args, format );// Initialize list of arguments.

    if( active_device == CONSOLE_OUTPUT ) {
    	vprintf( format, args );
    	fflush(stdout);
    } else if( active_device == TELNET_OUTPUT ) {

        uint16_t size = vsnprintf( NULL, 0, format, args ) + 1;// Get length with room for null terminator.

    	if( size > SPRINTF_BUFFER_LENGTH ) {
    		sprintf( buffer, "Message length(%u) for Telnet exceeds buffer length: %u\r\n", size, SPRINTF_BUFFER_LENGTH );
    		imx_printf( "Telnet Buffer Could Overflow with this message:\r\n");
    	    vprintf( format, args );
    	} else
    		vsprintf( buffer, format, args );
    	if( telnetd_write( buffer, strlen( buffer ) ) == false ) {
    		imx_printf( "Telnet stream failure, closing session\r\n" );
    		telnetd_deinit();
    	}
    }
    va_end( args );
}

int imx_log_printf( char *format, ... )
{
	if ( ( icb.print_debugs != 0 ) ) {
		char buffer[ SPRINTF_BUFFER_LENGTH ];
		va_list args;// Arguments from the expanded elipsis "..."
		wiced_utc_time_t utc_time = 0;

		wiced_time_get_utc_time( &utc_time );

	    va_start( args, format );// Initialize list of arguments.

    	uint16_t size = vsnprintf( NULL, 0, format, args ) + 1;	// Get length with room for null terminator.
		imx_printf( "%06lu: ", (uint32_t) ( utc_time % IMX_SEC_IN_DAY  ) );
	    vprintf( format, args );
	    fflush(stdout);

    	if( size > SPRINTF_BUFFER_LENGTH ) {
    		sprintf( buffer, "Message length(%u) for Telnet exceeds write buffer length: %u\r\n", size, SPRINTF_BUFFER_LENGTH );
    		imx_printf( "log buffer Could Overflow with this message: %s\r\n", buffer );
    	} else {
	    	if( telnet_active() == true ) {	// Output to telnet session as well
		    	sprintf( buffer, "%06lu: ", (uint32_t) ( utc_time % IMX_SEC_IN_DAY  ) );
		    	if( telnetd_write( buffer, strlen( buffer ) ) == false ) {
		    		imx_printf( "Telnet stream failure, closing session\r\n" );
		    		telnetd_deinit();
		    	}
	    	}
	    	vsprintf( buffer, format, args );
	    	if( telnet_active() == true ) {	// Output to telnet session as well
		    	if( telnetd_write( buffer, strlen( buffer ) ) == false ) {
		    		imx_printf( "Telnet stream failure, closing session\r\n" );
		    		telnetd_deinit();
		    	}
	    	}
//
//
//	    	if( ( icb.log_messages & DEBUG_LOG_TO_IMATRIX ) != 0 ) {
//	        	/*
//	        	 * Send log message to iMatrix
//	        	 */
//	    		vsprintf( buffer, format, args );
//	        	imatrix_log( buffer );
//	    	}
    	}
	    va_end( args );
	}
    return 0;
}
