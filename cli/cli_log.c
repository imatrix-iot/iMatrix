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
 * cli_dump.c
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "wiced.h"
#include "spi_flash.h"

#include "interface.h"
#include "cli_dump.h"
/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define CHAR_PER_LINE			32
#define MAX_BUFFER_SIZE			2048
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
extern sflash_handle_t sflash_handle;
/******************************************************
 *               Function Definitions
 ******************************************************/
/**
  * @brief
  * @param  None
  * @retval : None
  */
void cli_dump(uint16_t mode)
{
	uint8_t	*buffer;
	char *token, *foo;
	uint32_t dump_start, dump_length;
	/*
	 *	command format d [ <start address> ] [ <length> ] if no start, start at 0, if no length, print out 1k of data
	 */
	token = strtok(NULL, " " );	// Get start if any
	if( token ) {
		if( strncmp( token, "0x", 2 ) == 0 )
			dump_start = strtoul( &token[ 2 ], &foo, 16 );
		else
			dump_start = strtoul( token, &foo, 10 );
	}
	else
		dump_start = 0x00;
	token = strtok(NULL, " " );	// Get length
	if( token ) {
		if( strncmp( token, "0x", 2 ) == 0 )
			dump_length = strtoul( &token[ 2 ], &foo, 16 );
		else
			dump_length = strtoul( token, &foo, 10 );
	} else
		dump_length = MAX_BUFFER_SIZE;
	if( dump_length > MAX_BUFFER_SIZE )
		dump_length = MAX_BUFFER_SIZE;

	if( mode == DUMP_MEMORY ) {
		cli_print( "Dump of Internal Memory from: 0x%08x\r\n", dump_start );
		buffer = (uint8_t *) dump_start;
		hex_dump( buffer, dump_start, dump_length );
	} else {
		cli_print( "Dump of External Serial Flash Memory from: 0x%08x\r\n", dump_start );
		buffer = malloc( MAX_BUFFER_SIZE );
		if( buffer != 0 ) {

			dump_length = MAX_BUFFER_SIZE;
			sflash_read( &sflash_handle, dump_start, (void*) buffer, dump_length );
			/*
			 * Print the buffer out
			 */
			cli_print( "SFLASH Dump: 0x%08lx for %u Bytes\r\n", dump_start, dump_length );
			hex_dump( buffer, dump_start, dump_length );
		} else
			cli_print( "Could not allocate space for buffer\r\n" );
		free( buffer );
	}


}

void hex_dump( uint8_t *buffer, uint32_t dump_start, uint32_t dump_length )
{
	uint32_t i, j;

	i = 0;
	while( i < dump_length ) {
		/*
		 * Print 32 Bytes per line
		 */
		cli_print( "%08lX  ", dump_start + (uint32_t) i );
		for( j = 0; ( ( i + j ) < dump_length ) && ( j < CHAR_PER_LINE ); j++ ) {
			cli_print( "%02X ", buffer[ i + j ] );
			if( ( j == 7 ) || ( j == 15 ) || (j == 23 ) )	// Make it easier to read
				cli_print( " " );
		}
		for( j = 0; ( i + j ) < ( dump_length ) && ( j < CHAR_PER_LINE ); j++ )
			if( isprint( (uint16_t) buffer[ i + j ] ) && ( (uint16_t) buffer[ i + j ] != 0x0a ) && ((uint16_t) buffer[ i + j ] != 0x0d ) )
				cli_print( "%c", buffer[ i + j ] );
			else
				cli_print( "." );
		i += CHAR_PER_LINE;
		cli_print( "\r\n" );

	}
}
